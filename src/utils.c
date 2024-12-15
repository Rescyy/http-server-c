#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

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