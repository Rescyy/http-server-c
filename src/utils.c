//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <utils.h>
#include <alloc.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "logging.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/*
 * Find the first occurrence of find in s, where the search is limited to the
 * first slen characters of s.
 */
char *strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0')
    {
        len = strlen(find);
        do
        {
            do
            {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

int strindex(const char *str, const char *find)
{
    char *sub = strstr(str, find);
    if (sub == NULL)
        return -1;
    return sub - str;
}

// returns the index at which it is found, otherwise returns -1 if it is not found
int strnindex(const char *str, int n, const char *find)
{
    char *sub = strnstr(str, find, n);
    if (sub == NULL)
        return -1;
    return sub - str;
}

int strnindexAny(const char *str, int n, const char *find) {
    for (int i = 0; i < n; i++) {
        for (const char *c = find; *c != '\0'; c++) {
            if (*c == str[i]) {
                return i;
            }
        }
    }
    return -1;
}

int isAlpha(char c)
{
    c |= 0b00100000;
    return c >= 'a' && c <= 'z';
}

#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
+(uint32_t)(((const uint8_t *)(d))[0]) )

unsigned int hash(void *_data, int len)
{
    char *data = (char *)_data;
    unsigned int hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL)
        return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--)
    {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem)
    {
        case 3:
            hash += get16bits(data);
        hash ^= hash << 16;
        hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
        hash += hash >> 11;
        break;
        case 2:
            hash += get16bits(data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
        case 1:
            hash += (signed char)*data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

size_t getCurrentFormattedTime(char *buf, size_t size) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // convert to local time
    struct tm tm_info;
    gmtime_r(&ts.tv_sec, &tm_info);

    // format up to seconds
    size_t formattedSize = strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_info);
    // append milliseconds
    formattedSize += snprintf(buf + formattedSize, size - formattedSize, ".%03ld", ts.tv_nsec / 1000000);

    return formattedSize;
}

string copyString(string str) {
    if (str.length <= 0) {
        return str;
    }
    char *ptr = gcArenaAllocate(str.length + 1, sizeof(char));
    snprintf(ptr, str.length + 1, "%.*s", (int) str.length, str.ptr);
    return (string) {
        .ptr = ptr,
        .length = str.length,
    };
}

string copyStringFromSlice(const char *ptr, ssize_t len) {
    TRACE("len %zd %.*s", len, (int) len, ptr);
    if (len <= 0) {
        return (string) {.ptr = NULL, .length = len};
    }
    char *copyPtr = gcArenaAllocate(len + 1, sizeof(char));
    snprintf(copyPtr, len + 1, "%.*s", (int) len, ptr);
    return (string) {.ptr = copyPtr, .length = len};
}

ssize_t stringCompare(string *str1, string *str2) {
    ssize_t len = MIN(str1->length, str2->length);
    for (int i = 0; i < len; i++) {
        ssize_t diff = str1->ptr[i] - str2->ptr[i];
        if (diff) {
            return diff;
        }
    }
    return str1->length - str2->length;
}

char toLower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('A' - 'a');
    }
    return c;
}

ssize_t stringCompareIgnoreCase(string *str1, string *str2) {
    if (str1 == NULL || str2 == NULL) {
        return 0;
    }
    TRACE("stringCompareIgnoreCase %s %s", str1->ptr, str2->ptr);
    ssize_t len = MIN(str1->length, str2->length);
    for (int i = 0; i < len; i++) {
        TRACE("stringCompareIgnoreCase i=%d", i);
        ssize_t diff = toLower(str1->ptr[i]) - toLower(str2->ptr[i]);
        if (diff) {
            return diff;
        }
    }
    return str1->length - str2->length;
}

KeyValue *findKeyValue(KeyValue *keyValues, size_t count, const char *key) {
    if (keyValues == NULL || count == 0) {
        return NULL;
    }

    string keyString = (string) {.ptr = key, .length = (ssize_t) strlen(key)};

    TRACE("findKeyValue count=%zu", count);
    for (size_t i = 0; i < count; i++) {
        KeyValue *kv = &keyValues[i];
        TRACE("findKeyValue i=%d kv=%p", i, kv);
        if (stringCompareIgnoreCase(&kv->key, &keyString) == 0) {
            return kv;
        }
    }

    return NULL;
}
