//
// Created by Rescyy on 10/22/2025.
//

#ifndef HTTPSERVERC_PAGE_ARENAS_H
#define HTTPSERVERC_PAGE_ARENAS_H

#include <array.h>

#define ARENA_MAX_TRIES 3
#define ARENA_PAGE_CAP (1 << 12)    // 4 KB

typedef struct {
    void *ptr;
    size_t size;
    size_t capacity;
} ArenaChunk;

TYPEDEF_ARRAY(ArenaChunk);
DECLARE_ARRAY_FUNCS(ArenaChunk)

typedef struct {
    ARRAY_T(ArenaChunk) chunks;
    unsigned int openFrom;
    unsigned int lastSentFrom;
    unsigned int tries;
} Arena;

void cleanupArena(Arena *arena);
void destroyArena(Arena *arena);
void initArena();
void deInitArena();
Arena *getArena();
void setArena(const Arena *arena);
ArenaChunk newArenaChunk(size_t capacity);
void *chunkAlloc(ArenaChunk *chunk, size_t size, int align);
int chunkHasEnoughSpace(ArenaChunk *chunk, size_t, int align);

#endif //HTTPSERVERC_PAGE_ARENAS_H