//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef ALLOC_H
#define ALLOC_H

#include <stdio.h>

#define ALLOC_FILE "allocations.txt"
#define LEAK_FILE "leaks.txt"

// Public control for tracking lifecycle
void init_alloc(void);
void start_alloc_tracking(void);
void stop_alloc_tracking(void);
void *_allocate(size_t size);
void *_reallocate(void *ptr, size_t size);
void _deallocate(void *ptr);
void *_allocateTrack(size_t size, const char *file, int line);
void _deallocateTrack(void *ptr, const char *file, int line);
void *_reallocateTrack(void* ptr, size_t size, const char *file, int line);

#ifdef TRACK_LEAK
#define allocate(size)       _allocateTrack((size), __FILE__, __LINE__)
#define deallocate(ptr)      _deallocateTrack((ptr), __FILE__, __LINE__)
#define reallocate(ptr, sz)  _reallocateTrack((ptr), (sz), __FILE__, __LINE__)
#else
#define allocate(size)       _allocate((size))
#define deallocate(ptr)      _deallocate((ptr))
#define reallocate(ptr, sz)  _reallocate((ptr), (sz))
#endif

#endif // ALLOC_H
