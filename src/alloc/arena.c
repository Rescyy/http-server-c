//
// Created by Rescyy on 10/22/2025.
//

#include "arena.h"

#include <pthread.h>
#include <logging.h>

DEFINE_ARRAY_FUNCS(ArenaChunk, allocate, reallocate)

static pthread_key_t arenaThreadKey;

Arena *getArena() {
    Arena *arena = pthread_getspecific(arenaThreadKey);
    return arena;
}

void initArena() {
    pthread_key_create(&arenaThreadKey, NULL);
}

void deInitArena() {
    pthread_key_delete(arenaThreadKey);
}

void setArena(const Arena *arena) {
    pthread_setspecific(arenaThreadKey, arena);
}

void cleanupArena(Arena *arena) {
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

ArenaChunk newArenaChunk(size_t capacity) {
    size_t actualCapacity = ((capacity - 1) / ARENA_PAGE_CAP + 1) * ARENA_PAGE_CAP;
    return (ArenaChunk) {
        .ptr = allocate(actualCapacity),
        .size = 0,
        .capacity = actualCapacity,
    };
}

void destroyArena(Arena *arena) {
    debug("Cleaning up Arena %u Chunks", arena->chunks.length);
    for (size_t i = 0; i < arena->chunks.length; i++) {
        deallocate(arena->chunks.data[i].ptr);
    }
    deallocate(arena->chunks.data);
    deallocate(arena);
}

void *chunkAlloc(ArenaChunk *chunk, size_t size, unsigned int align) {
    chunk->size = ((chunk->size - 1) / align + 1) * align;
    void *returnPtr = (char*) chunk->ptr + chunk->size;
    chunk->size += size;
    return returnPtr;
}

int chunkHasEnoughSpace(ArenaChunk *chunk, size_t size, unsigned int align) {
    size_t chunkSize = chunk->size;
    if (chunk->size % align != 0) {
        chunkSize += align - chunk->size % align;
    }
    return chunkSize + size <= chunk->capacity;
}