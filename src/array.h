//
// Created by Crucerescu Vladislav on 17.08.2025.
//

#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "alloc.h"

// Macro to define array type
#define ARRAY_T(type) type##_array_t

#define TYPEDEF_ARRAY(type) \
typedef struct { \
    type *data; \
    unsigned int length; \
    unsigned int capacity; \
} ARRAY_T(type);

#define DECLARE_ARRAY_FUNCS(type) \
ARRAY_T(type) type##_array_with_capacity(unsigned int capacity);\
ARRAY_T(type) type##_array_new(void);\
void type##_array_ensure_capacity(ARRAY_T(type) *arr, unsigned int new_cap);\
void type##_array_push(ARRAY_T(type) *arr, type value);\
type type##_array_pop(ARRAY_T(type) *arr);\
unsigned int type##_array_length(ARRAY_T(type) *arr);\
void type##_array_push_range(ARRAY_T(type) *arr, type *range, int rangeLength);\
void type##_array_push_multiple(ARRAY_T(type) *arr, type element, int amount);

#define DEFINE_ARRAY_FUNCS(type) \
ARRAY_T(type) type##_array_with_capacity(unsigned int capacity) { \
    ARRAY_T(type) arr; \
    arr.data = (type *)allocate(capacity * sizeof(type)); \
    arr.length = 0; \
    arr.capacity = capacity; \
    return arr; \
} \
\
ARRAY_T(type) type##_array_new(void) { \
    return type##_array_with_capacity(1); \
} \
\
void type##_array_ensure_capacity(ARRAY_T(type) *arr, unsigned int new_cap) { \
    if (new_cap > arr->capacity) { \
        arr->capacity = (arr->capacity * 2 > new_cap) ? arr->capacity * 2 : new_cap; \
        arr->data = (type *)reallocate(arr->data, arr->capacity * sizeof(type)); \
    } \
} \
\
void type##_array_push(ARRAY_T(type) *arr, type value) { \
    if (arr->length >= arr->capacity) { \
        type##_array_ensure_capacity(arr, arr->length + 1); \
    } \
    arr->data[arr->length++] = value; \
} \
\
type type##_array_pop(ARRAY_T(type) *arr) { \
    if (arr->length == 0) { \
        fprintf(stderr, "Pop from empty array\n"); \
        exit(EXIT_FAILURE); \
    } \
    return arr->data[--arr->length]; \
} \
\
unsigned int type##_array_length(ARRAY_T(type) *arr) { \
    return arr->length; \
}\
\
type *type##_get_ptr(ARRAY_T(type) *arr) {\
    return arr->data;\
}\
\
void type##_array_push_range(ARRAY_T(type) *arr, type *range, int rangeLength) {\
    if (rangeLength <= 0) return;\
    type##_array_ensure_capacity(arr, arr->length + rangeLength);\
    memcpy(arr->data + arr->length, range, rangeLength * sizeof(type));\
    arr->length += rangeLength;\
}\
\
void type##_array_push_multiple(ARRAY_T(type) *arr, type value, int amount) {\
    if (amount <= 0) {return;}\
    type##_array_ensure_capacity(arr, arr->length + amount);\
    for (int i = 0; i < amount; i++) {\
        arr->data[arr->length + i] = value;\
    }\
    arr->length += amount;\
}

#define DEFINE_ARRAY_H(type) \
TYPEDEF_ARRAY(type) \
DECLARE_ARRAY_FUNCS(type)

#define DEFINE_ARRAY_C(type) \
DEFINE_ARRAY_FUNCS(type)

// Helper macros to call functions more nicely
#define ARRAY_NEW(type) type##_array_new()
#define ARRAY_WITH_CAPACITY(type, cap) type##_array_with_capacity(cap)
#define ARRAY_PUSH(type, arr, val) type##_array_push(arr, val)
#define ARRAY_POP(type, arr) type##_array_pop(arr)
#define ARRAY_LEN(type, arr) type##_array_length(arr)
#define ARRAY_ENSURE_CAPACITY(type, arr, cap) type##_array_ensure_capacity(arr, cap)
#define ARRAY_PUSH_RANGE(type, arr, range, rangeLength) type##_array_push_range(arr, range, rangeLength)
#define ARRAY_PUSH_MULTIPLE(type, arr, value, amount) type##_array_push_multiple(arr, value, amount)

#endif //ARRAY_H
