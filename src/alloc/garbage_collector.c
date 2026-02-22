//
// Created by Rescyy on 10/22/2025.
//

#include <alloc.h>

#include <pthread.h>

#include "alloc_entries.h"
#include "arena.h"
#include "destructors.h"
#include "logging.h"


void gcInit() {
    initEntries();
    initArena();
    initDestructors();
}

void gcDestroy() {
    deInitDestructors();
    deInitArena();
    deInitEntries();
}

static void destroyArenaWrapper(void *ptr) {
    destroyArena(ptr);
}

static void destroyEntriesWrapper(void *ptr) {
    destroyEntries(ptr);
}

void gcTrack() {
    gcTrackWithStackArena(NULL, 0);
}

void gcTrackWithStackArena(void *stackArenaChunk, size_t chunkSize) {
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
        .firstStackAlloc = stackArenaChunk != NULL,
    };
    if (stackArenaChunk != NULL) {
        ArenaChunk chunk = {.ptr = stackArenaChunk, .size = 0, .capacity = chunkSize};
        ARRAY_PUSH(ArenaChunk, &arena->chunks, chunk);
    } else {
        ARRAY_PUSH(ArenaChunk, &arena->chunks, newArenaChunk(ARENA_PAGE_CAP));
    }
    setArena(arena);

    ARRAY_T(Destructor) *destructors = allocate(SIZEOF_ARRAY);
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

void *gcArenaAllocate(const size_t size, unsigned int align) {
    assert(align != 0);
    if (size == 0) return NULL;

    Arena *arena = getArena();
    if (arena == NULL) {
        return allocate(size);
    }

    const int sizeInt = (int) size;

    if (size >= ARENA_PAGE_CAP) {
        ArenaChunk *chunk = &arena->chunks.data[arena->lastSentFrom];
        if (chunkHasEnoughSpace(chunk, size, align)) {
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
        if (chunkHasEnoughSpace(&arena->chunks.data[i], size, align)) {
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
        arena->tries = 0;
    }

    return returnPtr;
}

void gcArenaGiveBack(size_t size) {
    if (size == 0) {
        return;
    }
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
    if (ptr == NULL) {
        return gcAllocate(size);
    }
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
        ARRAY_PUSH(AllocEntry, &entries->arr, entry);
    }
    return newPtr;
}
