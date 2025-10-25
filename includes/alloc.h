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

void *allocate(size_t size);
void *reallocate(void *ptr, size_t size);
void deallocate(void *ptr);

// Initialises the GC engine.
// Call once in main.
void gcInit();

// Initialises allocations tracking of the gc allocation functions
// Call once per thread
void gcTrack();

// cleans up tracked gc allocations
void gcCleanup();

// de-initialises the GC engine
// Call after Init
void gcDestroy();

// Allocates bytes from the arena, the received memory cannot be grown dynamically
// The arena is tracked by the GC engine
void *gcArenaAllocate(size_t size, int align);

// Gives back some bytes from the previously allocated arena chunk.
// The behaviour is undefined if it is called
// with more than what was previously allocated.
// Can be called multiple times for the same
// allocation as long as it doesn't give more in total
// than the previous allocations.
void gcArenaGiveBack(size_t size);

// Allocates bytes, ptr is tracked by the GC engine
void *gcAllocate(size_t size);

// Reallocates bytes allocated by gcAllocate, ptr is tracked by the GC engine
void *gcReallocate(void *ptr, size_t size);

// Attach a custom destructor called at thread exit
void attachDestructor(destructor_t func, void *ptr);

#endif // ALLOC_H
