//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "alloc.h"

// ---------------- Tracking state ----------------

typedef struct {
    void *ptr;
    size_t size;
    char *locationHistory;
    int in_use;
} TrackEntry;

typedef struct {
    TrackEntry *g_entries;
    long threadId;
    size_t g_count;
    size_t g_cap;
} TrackArray;

pthread_key_t g_alloc_key;

static void exitIfOutOfMemory(const void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "The program is out of memory\n");
        exit(1);
    }
}

static long current_thread_id(void) {
    return (long)pthread_self();
}

static void ensure_capacity(TrackArray *trackArray, const size_t need) {
    size_t g_cap = trackArray->g_cap;
    if (g_cap >= need) return;
    size_t new_cap = g_cap ? g_cap * 2 : 128;
    while (new_cap < need) new_cap *= 2;
    trackArray->g_entries = _reallocate(trackArray->g_entries, new_cap * sizeof(TrackEntry));
    trackArray->g_cap = new_cap;
}

static TrackEntry *find_entry(const TrackArray *trackArray, const void *ptr) {
    TrackEntry *entries = trackArray->g_entries;
    for (size_t i = 0; i < trackArray->g_count; ++i) {
        if (entries[i].in_use && entries[i].ptr == ptr) return &entries[i];
    }
    return NULL;
}

static char *initLocationHistory(const char *path, const int line) {
    size_t locationHistorySize = strlen(path) + 20;
    char *locationHistory = _allocate(locationHistorySize);
    snprintf(locationHistory, locationHistorySize, "%s:%d", path, line);
    return locationHistory;
}

static char *appendLocationHistory(char *locationHistory, const char *path, int line) {
    size_t locationHistorySize = strlen(locationHistory) + strlen(path) + 20;
    char *newLocationHistory = _allocate(locationHistorySize);
    snprintf(newLocationHistory, locationHistorySize, "%s -> %s", locationHistory, initLocationHistory(path, line));
    _deallocate(locationHistory);
    return newLocationHistory;
}

static void track_add(void *ptr, size_t size, const char *file, int line) {
#ifdef TRACK_LEAK
    TrackArray *trackArray = pthread_getspecific(g_alloc_key);
    if (trackArray == NULL) return;
    ensure_capacity(trackArray, trackArray->g_count + 1);
    trackArray->g_entries[trackArray->g_count++] = (TrackEntry){ ptr, size, initLocationHistory(file, line), 1 };
#endif
}

static void track_remove(void *ptr) {
#ifdef TRACK_LEAK
    TrackArray *trackArray = pthread_getspecific(g_alloc_key);
    if (trackArray == NULL) return;
    TrackEntry *entry = find_entry(trackArray, ptr);
    if (entry != NULL) {
        entry->in_use = 0;
    }
#endif
}

static void track_update(void *old_ptr, void *new_ptr, size_t size, const char *file, int line) {
#ifdef TRACK_LEAK
    TrackArray *trackArray = pthread_getspecific(g_alloc_key);
    if (trackArray == NULL) return;
    TrackEntry *entry = find_entry(trackArray, old_ptr);
    if (entry != NULL) {
        entry->ptr = new_ptr;
        entry->size = size;
        char *locationHistory = entry->locationHistory;
        entry->locationHistory = appendLocationHistory(locationHistory, file, line);
    } else {
        printf("Tracker: old ptr not found when reallocating\n");
    }
#endif
}

static void dump_leaks(const TrackArray *trackArray) {
#ifdef LEAK_FILE
    FILE *logFile = NULL;
    logFile = fopen(LEAK_FILE, "a");
    TrackEntry *g_entries = trackArray->g_entries;

    for (size_t i = 0; i < trackArray->g_count; ++i) {
        if (g_entries[i].in_use) {
            if (logFile) {
                fprintf(logFile, "%ld,%s,%lu,%p\n",
                        trackArray->threadId,
                        g_entries[i].locationHistory,
                        (unsigned long)g_entries[i].size,
                        g_entries[i].ptr);
            }
            fprintf(stderr, "[LEAK] thread=%ld ptr=%p size=%lu at %s\n",
                    trackArray->threadId,
                    g_entries[i].ptr,
                    (unsigned long)g_entries[i].size,
                    g_entries[i].locationHistory);
            g_entries[i].in_use = 0;
        }
    }

    if (logFile) fclose(logFile);
#endif
}

void init_file(const char *path, const char *first_line) {
    FILE *f = fopen(ALLOC_FILE, "w");
    fwrite(first_line, 1, strlen(first_line), f);
    fclose(f);
}

void init_alloc(void) {
    pthread_key_create(&g_alloc_key, NULL);
    pthread_setspecific(g_alloc_key, NULL);
    init_file(ALLOC_FILE, "Type,ThreadID,File,Line,Size,Pointer\n");
    init_file(LEAK_FILE,"ThreadID,LocationHistory,Size,Pointer\n");
}

// Public control
void start_alloc_tracking(void) {
#if defined(TRACK_LEAK)
    TrackArray *trackArray = _allocate(sizeof(TrackArray));
    trackArray->g_cap = 1000;
    trackArray->g_entries = _allocate(sizeof(TrackEntry) * trackArray->g_cap);
    trackArray->g_count = 0;
    trackArray->threadId = current_thread_id();
    pthread_setspecific(g_alloc_key, trackArray);
#endif
}

void stop_alloc_tracking(void) {
#if defined(TRACK_LEAK)
    TrackArray *trackArray = pthread_getspecific(g_alloc_key);
    dump_leaks(trackArray);
    for (size_t i = 0; i < trackArray->g_count; ++i) {
        _deallocate(trackArray->g_entries[i].locationHistory);
    }
    _deallocate(trackArray->g_entries);
    _deallocate(trackArray);
    pthread_setspecific(g_alloc_key, NULL);
#endif
}

// ---------------- Logging backends ----------------

static void log_write(const char *kind, const char *file, int line, size_t size, void *ptr) {
#ifdef ALLOC_FILE
    FILE *logFile = fopen(ALLOC_FILE, "a");
    if (logFile) {
        long threadId = current_thread_id();
        if (size) {
            fprintf(logFile, "%s,%ld,%s,%d,%lu,%p\n",
                    kind, threadId, file, line, (unsigned long)size, ptr);
        } else {
            fprintf(logFile, "%s,%ld,%s,%d,,%p\n",
                    kind, threadId, file, line, ptr);
        }
        fclose(logFile);
    }
#else
    (void)kind; (void)file; (void)line; (void)size; (void)ptr;
#endif
}

void *_allocateTrack(size_t size, const char *file, int line)
{
    void *ptr = _allocate(size);
    track_add(ptr, size, file, line);
    log_write("Alloc", file, line, size, ptr);
    return ptr;
}

void _deallocateTrack(void *ptr, const char *file, int line)
{
    log_write("Free", file, line, 0, ptr);
    track_remove(ptr);
    _deallocate(ptr);
}

void *_reallocateTrack(void *ptr, size_t size, const char *file, int line)
{
    void *newPtr = _reallocate(ptr, size);
    log_write("Free",  file, line, 0, ptr);
    track_update(ptr, newPtr, size, file, line);
    log_write("Alloc", file, line, size, newPtr);
    return newPtr;
}

// ---------------- Plain backends ----------------

void *_allocate(size_t size)
{
    void *ptr = malloc(size);
    exitIfOutOfMemory(ptr);
    return ptr;
}

void *_reallocate(void *ptr, size_t size)
{
    void *newPtr = realloc(ptr, size);
    exitIfOutOfMemory(newPtr);
    return newPtr;
}

void _deallocate(void *ptr)
{
    free(ptr);
}
