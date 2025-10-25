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
        ARRAY_PUSH(AllocEntry, &entries->arr, entry);
    }
    return newPtr;
}
