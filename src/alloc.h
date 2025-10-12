//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef ALLOC_H
#define ALLOC_H

#include <stdio.h>
#include "array.h"

#define ALLOC_FILE "allocations.txt"
#define LEAK_FILE "leaks.txt"

typedef void (*destructor_t) (void*);

// #ifdef MEMORY_COLLECTION
void *allocate(size_t size);
void *reallocate(void *ptr, size_t size);
void deallocate(void *ptr);
void gcInit(); // initialises the thread key
void gcTrack(); // initialises thread specific tracking data
void gcCleanup(); // cleans up the thread data and re-initialises it
void *gcArenaAllocate(size_t size, int align); // allocates bytes from the arena, aligned to 8 bytes
void gcArenaGiveBack(size_t size); // takes back some of the given data from the most recent used chunk
void *gcAllocate(size_t size); // allocates bytes and adds an allocation entry
void *gcReallocate(void *ptr, size_t size); // reallocates bytes and adds an reallocation entry
void attachDestructor(destructor_t func, void *ptr);

#endif // ALLOC_H
