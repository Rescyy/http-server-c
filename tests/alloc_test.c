//
// Created by Rescyy on 10/26/2025.
//

#include "test.h"
#include "alloc.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#define SMALL_SIZE 16
#define MEDIUM_SIZE 256
#define LARGE_SIZE 4096
#define ARENA_PAGE_CAP 4096

// =============== Helper Thread ===============
void* threadRoutine(void*) {
    long long testResult = 1;
    gcTrack();
    void* a1 = gcAllocate(SMALL_SIZE);
    EXPECT(a1 != NULL);
    void* a2 = gcReallocate(a1, SMALL_SIZE * 2);
    EXPECT(a2 != NULL);
    void* a3 = gcArenaAllocate(MEDIUM_SIZE, 8);
    EXPECT(a3 != NULL);
    gcArenaGiveBack(8);
    gcCleanup();
    return (void*) testResult;
}

// =============== Basic Allocator Tests ===============
int test1_allocate_basic() {
    int testResult = 1;
    
    void* p = allocate(8);
    EXPECT(p != NULL);
    deallocate(p);
    return testResult;
}

int test2_allocate_zero() {
    int testResult = 1;
    
    void* p = allocate(0);
    EXPECT(p == NULL);
    return testResult;
}

int test3_reallocate_null() {
    int testResult = 1;
    
    void* p = reallocate(NULL, 32);
    EXPECT(p != NULL);
    deallocate(p);
    return testResult;
}

int test4_reallocate_zero() {
    int testResult = 1;
    
    void* p = reallocate(NULL, 0);
    EXPECT(p == NULL);
    return testResult;
}

int test5_reallocate_grow() {
    int testResult = 1;
    
    void* p = allocate(8);
    p = reallocate(p, 64);
    EXPECT(p != NULL);
    deallocate(p);
    return testResult;
}

int test6_deallocate_null() {
    int testResult = 1;
    
    deallocate(NULL);
    EXPECT(1);
    return testResult;
}

// =============== GC Lifecycle ===============
int test7_gc_init_destroy() {
    int testResult = 1;
    
    gcInit();
    gcDestroy();
    EXPECT(1);
    return testResult;
}

int test8_gc_track_and_cleanup() {
    int testResult = 1;
    
    gcInit();
    gcTrack();
    gcCleanup();
    gcDestroy();
    EXPECT(1);
    return testResult;
}

int test9_gc_double_init_destroy() {
    int testResult = 1;
    
    gcInit();
    gcDestroy();
    gcInit();
    gcDestroy();
    EXPECT(1);
    return testResult;
}

int test10_gc_track_multiple_threads() {
    long long testResult1;
    long long testResult2;

    gcInit();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, threadRoutine, NULL);
    pthread_create(&t2, NULL, threadRoutine, NULL);
    pthread_join(t1, (void **) &testResult1);
    pthread_join(t2, (void **) &testResult2);
    gcDestroy();

    return testResult1 && testResult2;
}

// =============== Arena Tests ===============
int test11_arena_allocate_small() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* ptr = gcArenaAllocate(32, 4);
    EXPECT(ptr != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test12_arena_allocate_large() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* ptr = gcArenaAllocate(ARENA_PAGE_CAP * 2, 8);
    EXPECT(ptr != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test13_arena_allocate_alignment() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* p = gcArenaAllocate(64, 16);
    EXPECT((uintptr_t)p % 16 == 0);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test14_arena_giveback_exact() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    gcArenaAllocate(128, 8);
    gcArenaGiveBack(64);
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

int test15_arena_fill_many_chunks() {
    int testResult = 1;

    srand(1);

    gcInit(); gcTrack();
    for (int i = 0; i < 1000; i++) {
        EXPECT(gcArenaAllocate(rand() % (4 * LARGE_SIZE), 1 << (rand() % 5)) != NULL);
    }
    gcCleanup(); gcDestroy();
    return testResult;
}

// =============== GC Allocation Tracking ===============
int test16_gc_allocate() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* p = gcAllocate(32);
    EXPECT(p != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test17_gc_reallocate_newptr() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* p1 = gcAllocate(32);
    void* p2 = gcReallocate(p1, 64);
    EXPECT(p2 != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test18_gc_reallocate_sameptr() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* p = gcAllocate(16);
    p = gcReallocate(p, 16);
    EXPECT(p != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test19_gc_cleanup_frees_all() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    for (int i = 0; i < 100; i++) gcAllocate(32);
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

int test20_gc_cleanup_arena() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    gcArenaAllocate(256, 8);
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

void dtor(void* p) { *(int*)p = 1; }

// =============== Destructors ===============
int test21_attach_destructor_called() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    int flag = 0;
    attachDestructor(dtor, &flag);
    gcDestroy();
    EXPECT(flag == 1);
    return testResult;
}

void d1(void* p){*(int*)p=1;}
void d2(void* p){*(int*)p=1;}

int test22_multiple_destructors_called() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    int f1 = 0, f2 = 0;
    attachDestructor(d1, &f1);
    attachDestructor(d2, &f2);
    gcDestroy();
    EXPECT(f1 == 1 && f2 == 1);
    return testResult;
}

// =============== Edge & Stress ===============
int test23_arena_giveback_zero() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    gcArenaAllocate(128, 8);
    gcArenaGiveBack(0);
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

int test24_gc_reallocate_nullptr() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    void* p = gcReallocate(NULL, 64);
    EXPECT(p != NULL);
    gcCleanup(); gcDestroy();
    return testResult;
}

int test25_large_allocs_stress() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    for (int i = 0; i < 50; i++) {
        EXPECT(gcArenaAllocate(LARGE_SIZE, 16) != NULL);
    }
    gcCleanup(); gcDestroy();
    return testResult;
}

int test26_mixed_gc_and_arena_allocations() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    for (int i = 0; i < 50; i++) {
        gcAllocate(64);
        gcArenaAllocate(128, 8);
    }
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

int test27_repeated_cleanup() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    gcCleanup();
    gcCleanup();
    gcDestroy();
    EXPECT(1);
    return testResult;
}

int test28_multithread_cleanup() {
    long long testResult;

    gcInit();
    pthread_t t;
    pthread_create(&t, NULL, threadRoutine, NULL);
    pthread_join(t, (void**) &testResult);
    gcDestroy();

    return testResult;
}

int test29_gc_destroy_no_track() {
    int testResult = 1;
    
    gcInit();
    gcDestroy();
    EXPECT(1);
    return testResult;
}

int test30_allocation_pattern_mix() {
    int testResult = 1;
    
    gcInit(); gcTrack();
    for (int i = 0; i < 200; i++) {
        if (i % 3 == 0) gcAllocate(32);
        else if (i % 3 == 1) gcArenaAllocate(64, 8);
        else gcReallocate(NULL, 128);
    }
    gcCleanup(); gcDestroy();
    EXPECT(1);
    return testResult;
}

int test31_arena_fill_many_chunks_and_use() {
    int testResult = 1;

    srand(1);

    gcInit(); gcTrack();
    for (int i = 0; i < 5000; i++) {

        int align;
        if (rand() % 2) {
            align = (1 << (rand() % 4 + 1)) * (rand() % 4 + 1);
        } else {
            align = 1;
        }

        size_t elements = (rand() % 16 + 1) * (rand() % 16 + 1);
        size_t size = elements * align;

        void *ptr = gcArenaAllocate(size, align);
        EXPECT(ptr != NULL || size == 0);
        memset(ptr, 0xff, size);

        gcArenaGiveBack((rand() % elements) * align);
    }
    gcCleanup(); gcDestroy();
    return testResult;
}

// =============== MAIN RUNNER ===============
int main() {
    INIT_UNIT_TESTS

    UNIT_TEST(test1_allocate_basic)
    UNIT_TEST(test2_allocate_zero)
    UNIT_TEST(test3_reallocate_null)
    UNIT_TEST(test4_reallocate_zero)
    UNIT_TEST(test5_reallocate_grow)
    UNIT_TEST(test6_deallocate_null)
    UNIT_TEST(test7_gc_init_destroy)
    UNIT_TEST(test8_gc_track_and_cleanup)
    UNIT_TEST(test9_gc_double_init_destroy)
    UNIT_TEST(test10_gc_track_multiple_threads)
    UNIT_TEST(test11_arena_allocate_small)
    UNIT_TEST(test12_arena_allocate_large)
    UNIT_TEST(test13_arena_allocate_alignment)
    UNIT_TEST(test14_arena_giveback_exact)
    UNIT_TEST(test15_arena_fill_many_chunks)
    UNIT_TEST(test16_gc_allocate)
    UNIT_TEST(test17_gc_reallocate_newptr)
    UNIT_TEST(test18_gc_reallocate_sameptr)
    UNIT_TEST(test19_gc_cleanup_frees_all)
    UNIT_TEST(test20_gc_cleanup_arena)
    UNIT_TEST(test21_attach_destructor_called)
    UNIT_TEST(test22_multiple_destructors_called)
    UNIT_TEST(test23_arena_giveback_zero)
    UNIT_TEST(test24_gc_reallocate_nullptr)
    UNIT_TEST(test25_large_allocs_stress)
    UNIT_TEST(test26_mixed_gc_and_arena_allocations)
    UNIT_TEST(test27_repeated_cleanup)
    UNIT_TEST(test28_multithread_cleanup)
    UNIT_TEST(test29_gc_destroy_no_track)
    UNIT_TEST(test30_allocation_pattern_mix)
    UNIT_TEST(test31_arena_fill_many_chunks_and_use)

    TEST_RESULTS
    return failed;
}
