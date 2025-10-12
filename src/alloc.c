//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>

#include "alloc.h"

#include <unistd.h>

#include "app.h"
#include "logging.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

static void exitIfOutOfMemory(const void *ptr) {
    if (ptr != NULL) {
        return;
    }
    pthread_t threadId = pthread_self();
    if (pthread_equal(threadId, getMainThreadId())) {
        fprintf(stderr, "The program is out of memory\n");
        exit(1);
    }
    fprintf(stderr, "The thread %lu did not receive the required memory\n", (unsigned long) threadId);
    pthread_exit(NULL);
}

void *allocate(size_t size)
{
    if (size == 0) return NULL;
    fflush(stdout);
    void *ptr = malloc(size);
    fflush(stdout);
    exitIfOutOfMemory(ptr);
    return ptr;
}

void *reallocate(void *ptr, size_t size)
{
    if (size == 0) return NULL;
    if (ptr == NULL) return allocate(size);
    void *newPtr = realloc(ptr, size);
    exitIfOutOfMemory(newPtr);
    return newPtr;
}

void deallocate(void *ptr)
{
    if (ptr == NULL) return;
    free(ptr);
}

typedef struct {
    void *ptr;
    size_t size;
    size_t capacity;
} ArenaChunk;

TYPEDEF_ARRAY(ArenaChunk);
DEFINE_ARRAY_FUNCS(ArenaChunk, allocate, reallocate)

typedef struct {
    ARRAY_T(ArenaChunk) chunks;
    unsigned int openFrom;
    unsigned int lastSentFrom;
    unsigned int tries;
} Arena;

typedef struct {
    void *ptr;
    void *prevPtr;
} AllocEntry;

TYPEDEF_ARRAY(AllocEntry);
DEFINE_ARRAY_FUNCS(AllocEntry, allocate, reallocate)

typedef struct {
    ARRAY_T(AllocEntry) arr;
    int toDeallocate;
} AllocEntries;

typedef struct {
    void (*func)(void *ptr);
    void *ptr;
} Destructor;

TYPEDEF_ARRAY(Destructor);
DEFINE_ARRAY_FUNCS(Destructor, allocate, reallocate)

static pthread_key_t arenaThreadKey;
static pthread_key_t entriesThreadKey;
static pthread_key_t destructorsThreadKey;

static void cleanupEntries(AllocEntries *entries);
static void cleanupArena(Arena *arena);
static void destroyEntries(AllocEntries *entries);
static void destroyArena(Arena *arena);
static void bundleEntriesAndDeallocate(const AllocEntries *entries);
static ArenaChunk newArenaChunk(size_t capacity);
static void invokeDestructors(ARRAY_T(Destructor) *destructors);

static Arena *getArena() {
    Arena *arena = pthread_getspecific(arenaThreadKey);
    return arena;
}

static AllocEntries *getEntries() {
    AllocEntries *entries = pthread_getspecific(entriesThreadKey);
    return entries;
}

static ARRAY_T(Destructor) *getDestructors() {
    ARRAY_T(Destructor) *destructors = pthread_getspecific(destructorsThreadKey);
    return destructors;
}

static void setArena(const Arena *arena) {
    pthread_setspecific(arenaThreadKey, arena);
}

static void setEntries(const AllocEntries *entries) {
    pthread_setspecific(entriesThreadKey, entries);
}

static void setDestructors(const ARRAY_T(Destructor) *destructors) {
    pthread_setspecific(destructorsThreadKey, destructors);
}

static void destroyArenaWrapper(void *ptr) {
    destroyArena(ptr);
}

static void destroyEntriesWrapper(void *ptr) {
    destroyEntries(ptr);
}

static void invokeDestructorsWrapper(void *ptr) {
    invokeDestructors(ptr);
}

void gcInit() {
    pthread_key_create(&entriesThreadKey, NULL);
    pthread_key_create(&arenaThreadKey, NULL);
    pthread_key_create(&destructorsThreadKey, invokeDestructorsWrapper);

}

#define ARENA_MAX_TRIES 3
#define ARENA_PAGE_CAP (1 << 12)    // 4 KB
#define ENTRIES_PAGE_CAP (1 << 8)   // 4 KB / 16 B

void gcTrack() {
    AllocEntries *entries = allocate(sizeof(AllocEntries));
    *entries = (AllocEntries) {
        .arr = ARRAY_WITH_CAPACITY(AllocEntry, ENTRIES_PAGE_CAP),
        .toDeallocate = 0,
    };
    setEntries(entries);

    Arena *arena = allocate(sizeof(Arena));
    *arena = (Arena) {
        .chunks = ARRAY_NEW(ArenaChunk),
        .openFrom = 0,
    };
    ARRAY_PUSH(ArenaChunk, &arena->chunks, newArenaChunk(ARENA_PAGE_CAP));
    ARRAY_T(Destructor) *destructors = allocate(SIZEOF_ARRAY);
    setArena(arena);

    *destructors = ARRAY_NEW(Destructor);
    setDestructors(destructors);

    attachDestructor(destroyEntriesWrapper, entries);
    attachDestructor(destroyArenaWrapper, arena);
}

void gcCleanup() {
    debug("Cleaning up allocations");
    cleanupEntries(getEntries());
    cleanupArena(getArena());
}

static void *chunkAlloc(ArenaChunk *chunk, size_t size, int align) {
    chunk->size = ((chunk->size - 1) / align + 1) * align;
    void *returnPtr = (char*) chunk->ptr + chunk->size;
    chunk->size += size;
    return returnPtr;
}

void *gcArenaAllocate(const size_t size, int align) {
    Arena *arena = getArena();
    if (arena == NULL) {
        return allocate(size);
    }

    const int sizeInt = (int) size;

    if (size >= ARENA_PAGE_CAP) {
        ArenaChunk *chunk = &arena->chunks.data[arena->chunks.length - 1];
        if (chunk->capacity >= chunk->size + size) {
            return chunkAlloc(chunk, sizeInt, align);
        }
        ARRAY_PUSH(ArenaChunk, &arena->chunks, newArenaChunk(size));
        chunk = &arena->chunks.data[arena->chunks.length - 1];
        arena->lastSentFrom = arena->chunks.length - 1;
        chunk->size = sizeInt;
        return chunk->ptr;
    }

    void *returnPtr = NULL;

    for (unsigned int i = arena->openFrom; i < arena->chunks.length; i++) {
        if (arena->chunks.data[i].capacity - arena->chunks.data[i].size >= size) {
            returnPtr = chunkAlloc(&arena->chunks.data[i], size, align);
            arena->lastSentFrom = i;
            break;
        }
    }

    if (returnPtr == NULL) {
        ARRAY_PUSH(ArenaChunk, &arena->chunks, newArenaChunk(ARENA_PAGE_CAP));
        ArenaChunk *chunk = &arena->chunks.data[arena->chunks.length - 1];
        chunk->size = sizeInt;
        returnPtr = chunk->ptr;
        arena->lastSentFrom = arena->chunks.length - 1;
    }

    if (arena->lastSentFrom == arena->openFrom) {
        arena->tries = 0;
    } else {
        arena->tries++;
    }

    if (arena->tries >= ARENA_MAX_TRIES) {
        arena->openFrom = arena->lastSentFrom;
    }

    return returnPtr;
}

void gcArenaGiveBack(size_t size) {
    Arena *arena = getArena();
    if (arena == NULL) {
        return;
    }
    ArenaChunk *chunk = &arena->chunks.data[arena->lastSentFrom];
    assert(chunk->size >= size);
    chunk->size -= size;
}

void *gcAllocate(size_t size) {
    void *ptr = allocate(size);
    AllocEntries *entries = getEntries();
    if (entries == NULL) {
        return ptr;
    }
    AllocEntry entry = {
        .ptr = ptr,
        .prevPtr = NULL,
    };
    ARRAY_PUSH(AllocEntry, &entries->arr, entry);
    entries->toDeallocate++;
    return ptr;
}

void *gcReallocate(void *ptr, size_t size) {
    void *newPtr = reallocate(ptr, size);
    AllocEntries *entries = getEntries();
    if (entries == NULL) {
        return newPtr;
    }
    if (newPtr != ptr) {
        AllocEntry entry = {
            .ptr = newPtr,
            .prevPtr = ptr,
        };
        AllocEntries *entries = getEntries();
        ARRAY_PUSH(AllocEntry, &entries->arr, entry);
    }
    return newPtr;
}

void attachDestructor(destructor_t func, void *ptr) {
    Destructor destructor = {
        .func = func,
        .ptr = ptr,
    };
    ARRAY_T(Destructor) *destructors = getDestructors();
    ARRAY_PUSH(Destructor, destructors, destructor);
}

void invokeDestructors(ARRAY_T(Destructor) *destructors) {
    debug("Invoking destructors %u", destructors->length);
    for (int i = destructors->length - 1; i >= 0; i--) {
        destructors->data[i].func(destructors->data[i].ptr);
    }
    deallocate(destructors->data);
    deallocate(destructors);
}

static void cleanupEntries(AllocEntries *entries) {
    debug("Cleaning up Entries %u to deallocate %u", entries->arr.length, entries->toDeallocate);
    bundleEntriesAndDeallocate(entries);
    ARRAY_RESIZE(AllocEntry, &entries->arr, 0, ENTRIES_PAGE_CAP);
    entries->toDeallocate = 0;
}

static void cleanupArena(Arena *arena) {
    debug("Cleaning up Arena %u Chunks", arena->chunks.length);
    for (size_t i = 1; i < arena->chunks.length; i++) {
        deallocate(arena->chunks.data[i].ptr);
    }
    ARRAY_RESIZE(ArenaChunk, &arena->chunks, 1, 1);
    arena->chunks.data[0].size = 0;
    arena->lastSentFrom = 0;
    arena->openFrom = 0;
    arena->tries = 0;
}

static void destroyEntries(AllocEntries *entries) {
    debug("Cleaning up Entries %u to deallocate %u", entries->arr.length, entries->toDeallocate);
    bundleEntriesAndDeallocate(entries);
    deallocate(entries->arr.data);
    deallocate(entries);
}

static void destroyArena(Arena *arena) {
    debug("Cleaning up Arena %u Chunks", arena->chunks.length);
    for (size_t i = 0; i < arena->chunks.length; i++) {
        deallocate(arena->chunks.data[i].ptr);
    }
    deallocate(arena->chunks.data);
    deallocate(arena);
}

static size_t ptrHashFunction(void *ptr) {
    size_t asSizeT = (size_t) ptr;
    return asSizeT >> 7 ^ asSizeT >> 4;
}

static void bundleEntriesAndDeallocate(const AllocEntries *entries) {
    const ARRAY_T(AllocEntry) *arr = &entries->arr;
    const size_t setCapacity = entries->toDeallocate * 2;
    void **allocPtr = calloc(setCapacity, sizeof(void *));
    for (size_t i = 0; i < arr->length; i++) {
        void *prevPtr = arr->data[i].prevPtr;
        void *ptr = arr->data[i].ptr;
        if (prevPtr != NULL) {
            size_t prevPtrHash = ptrHashFunction(prevPtr) % setCapacity;
            while (allocPtr[prevPtrHash] != prevPtr) {
                prevPtrHash = (prevPtrHash + 1) % setCapacity;
            }
            allocPtr[prevPtrHash] = NULL;
        }
        size_t ptrHash = ptrHashFunction(ptr) % setCapacity;
        while (allocPtr[ptrHash] != NULL) {
            ptrHash = (ptrHash + 1) % setCapacity;
        }
        allocPtr[ptrHash] = ptr;
    }
    for (size_t i = 0; i < setCapacity; i++) {
        deallocate(allocPtr[i]);
    }
    free(allocPtr);
}

static ArenaChunk newArenaChunk(size_t capacity) {
    size_t actualCapacity = ((capacity - 1) / ARENA_PAGE_CAP + 1) * ARENA_PAGE_CAP;
    return (ArenaChunk) {
        .ptr = allocate(actualCapacity),
        .size = 0,
        .capacity = actualCapacity,
    };
}
