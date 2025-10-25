//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "../includes/utils.h"
#include "../includes/alloc.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

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

int strnindex(const char *str, int n, const char *find)
{
    char *sub = strnstr(str, find, n);
    if (sub == NULL)
        return -1;
    return sub - str;
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
    void *ptr = gcArenaAllocate(str.length + 1, sizeof(char));
    snprintf(ptr, str.length + 1, "%.*s", (int) str.length, str.ptr);
    return (string) {
        .ptr = ptr,
        .length = str.length,
    };
}