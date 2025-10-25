//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <alloc.h>
#include <app.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

static void exitIfOutOfMemory(const void *ptr);

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