//
// Created by Rescyy on 10/22/2025.
//

#ifndef HTTPSERVERC_ALLOC_ENTRIES_H
#define HTTPSERVERC_ALLOC_ENTRIES_H

#include <array.h>

#define ENTRIES_PAGE_CAP (1 << 8)   // 4 KB / 16 B

typedef struct {
    void *ptr;
    void *prevPtr;
} AllocEntry;

TYPEDEF_ARRAY(AllocEntry);
DECLARE_ARRAY_FUNCS(AllocEntry)

typedef struct {
    ARRAY_T(AllocEntry) arr;
    int toDeallocate;
} AllocEntries;

void initEntries();
void deInitEntries();
void cleanupEntries(AllocEntries *entries);
void destroyEntries(AllocEntries *entries);
void bundleAndDeallocate(const AllocEntries *entries);
AllocEntries *getEntries();
void setEntries(const AllocEntries *entries);

#endif //HTTPSERVERC_ALLOC_ENTRIES_H