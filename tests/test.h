//
// Created by Rescyy on 10/23/2025.
//

#ifndef HTTPSERVERC_TEST_H
#define HTTPSERVERC_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int debug_ = 0;
#define DEBUG                        \
    printf("Debug %d\n", debug_); \
    debug_++;                     \
    fflush(stdout);

#define EXPECT(x)                              \
    if (!(x))                                  \
    {                                          \
        printf("LINE: %d %s\n", __LINE__, #x); \
        testResult = 0;                        \
    }

#define INIT_UNIT_TESTS               \
    int passed = 0, total = 0;        \
    printf("\nRunning Tests...\n\n"); \
    printf("------------------------------------------------------------------------------\n");
#define TEST_RESULTS printf("\nTests passed %d/%d.\n\n", passed, total); int failed = total - passed;

#define UNIT_TEST(x)                                                                            \
    printf("Running %s...\n\n", #x);                                                            \
    int testResult##x = x();                                                                    \
    passed += testResult##x;                                                                    \
    total++;                                                                                    \
    if (testResult##x)                                                                          \
    {                                                                                           \
        printf("\nPassed\n");                                                                   \
    }                                                                                           \
    else                                                                                        \
    {                                                                                           \
        printf("\nFailed\n");                                                                   \
    }                                                                                           \
    printf("------------------------------------------------------------------------------\n"); \
    fflush(stdout);


#endif //HTTPSERVERC_TEST_H