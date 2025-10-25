//
// Created by Rescyy on 10/22/2025.
//

#include <pthread.h>
#include <logging.h>

#include "alloc_entries.h"

DEFINE_ARRAY_FUNCS(AllocEntry, allocate, reallocate)

static pthread_key_t entriesThreadKey;

static size_t ptrHashFunction(const void *ptr);

void initEntries() {
    pthread_key_create(&entriesThreadKey, NULL);
}

void deInitEntries() {
    pthread_key_delete(entriesThreadKey);
}

AllocEntries *getEntries() {
    AllocEntries *entries = pthread_getspecific(entriesThreadKey);
    return entries;
}

void setEntries(const AllocEntries *entries) {
    pthread_setspecific(entriesThreadKey, entries);
}

void destroyEntries(AllocEntries *entries) {
    bundleAndDeallocate(entries);
    deallocate(entries->arr.data);
    deallocate(entries);
}

void bundleAndDeallocate(const AllocEntries *entries) {
    debug("Cleaning up Entries %u to deallocate %u", entries->arr.length, entries->toDeallocate);

    const ARRAY_T(AllocEntry) *arr = &entries->arr;
    if (entries->toDeallocate == 0) {
        return;
    }
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

void cleanupEntries(AllocEntries *entries) {
    bundleAndDeallocate(entries);
    ARRAY_RESIZE(AllocEntry, &entries->arr, 0, ENTRIES_PAGE_CAP);
    entries->toDeallocate = 0;
}

static size_t ptrHashFunction(const void *ptr) {
    const size_t asSizeT = (size_t) ptr;
    return asSizeT >> 7 ^ asSizeT >> 4;
}
