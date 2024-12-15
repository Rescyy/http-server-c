#include "http.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int parseHeaders(HttpHeaders *headers, const char *str, int size)
{
    int offset, prevOffset = 0, headerCap = 1;
    headers->arr = malloc(sizeof(HttpHeader));
    headers->count = 0;

    while (strncmp(str + prevOffset, "\r\n", 2) != 0)
    {
        if (headers->count >= headerCap)
        {
            headerCap *= 2;
            headers->arr = realloc(headers->arr, headerCap * sizeof(HttpHeader));
        }
        offset = strnindex(str + prevOffset, size - prevOffset, ": ");
        if (offset == -1 ||
            strnindex(str + prevOffset, offset, "\r") != -1 ||
            strnindex(str + prevOffset, offset, "\n") != -1)
        {
            freeHeaders(headers);
            return -1;
        }
        char *key = malloc(offset + 1);
        headers->arr[headers->count].key = key;
        snprintf(key, offset + 1, "%s", str + prevOffset);
        prevOffset += offset + 2;
        if (str[prevOffset] == ' ')
        {
            freeHeaders(headers);
            free(key);
            return -1;
        }

        offset = strnindex(str + prevOffset, size - prevOffset, "\r\n");
        if (offset == -1 ||
            strnindex(str + prevOffset, offset, "\r") != -1 ||
            strnindex(str + prevOffset, offset, "\n") != -1)
        {
            free(key);
            freeHeaders(headers);
            return -1;
        }
        char *value = malloc(offset + 1);
        headers->arr[headers->count].value = value;
        snprintf(value, offset + 1, "%s", str + prevOffset);
        prevOffset += offset + 2;

        headers->count++;
    }

    return prevOffset + 2;
}

void freeHeaders(HttpHeaders *headers)
{
    for (int i = 0; i < headers->count; i++)
    {
        free(headers->arr[i].key);
        free(headers->arr[i].value);
    }
    free(headers->arr);
    headers->arr = NULL;
    headers->count = 0;
}

int headerEq(HttpHeader obj1, HttpHeader obj2)
{
    return strcasecmp(obj1.key, obj2.key) == 0 && strcmp(obj1.value, obj2.value) == 0;
}

int headersEq(HttpHeaders obj1, HttpHeaders obj2)
{
    if (obj1.count != obj2.count)
    {
        return 0;
    }
    for (int i = 0; i < obj1.count; i++)
    {
        HttpHeader *header = findHeader(obj2, obj1.arr[i].key);
        
        if (header == NULL || strcmp(header->value, obj1.arr[i].value) != 0)
        {
            return 0;
        }
    }
    return 1;
}

int isVersionValid(const char *str, int n)
{
#define IS_EQUAL(str, n, ver)      \
    if (strncmp(str, ver, n) == 0) \
        return 1;
    IS_EQUAL(str, n, "HTTP/1.0");
    IS_EQUAL(str, n, "HTTP/1.1");
    IS_EQUAL(str, n, "HTTP/2.0");
    return 0;
}

HttpHeader *findHeader(HttpHeaders headers, const char *key)
{
    for (int i = 0; i < headers.count; i++)
    {
        if (strcasecmp(headers.arr[i].key, key) == 0)
        {
            return &headers.arr[i];
        }
    }
    return NULL;
}