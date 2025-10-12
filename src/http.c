//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "http.h"
#include <string.h>
#include <stdio.h>
#include "alloc.h"
#include "utils.h"

#define INITIAL_HEADER_CAP 8
#define MAX_HEADER_SIZE 8192

int parseHeadersStream(HttpHeaders *headers, TcpStream *stream)
{
    int headerCap = INITIAL_HEADER_CAP;
    headers->arr = gcAllocate(sizeof(HttpHeader) * headerCap);
    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + 2);
        if (stream->error != 0)
        {
            return stream->error;
        }
        if(strncmp(stream->buffer + stream->cursor, "\r\n", 2) == 0) {
            break;
        }
        string headerKey = tcpStreamReadUntilString(stream, MAX_HEADER_SIZE, ": ", 2);
        if (headerKey.length < 0)
        {
            return headerKey.length;
        }
        string headerValue = tcpStreamReadUntilCRLF(stream, MAX_HEADER_SIZE, 0);
        if (headerKey.length < 0)
        {
            return headerValue.length;
        }
        if (headers->count >= headerCap)
        {
            headerCap *= 2;
            headers->arr = gcReallocate(headers->arr, headerCap * sizeof(HttpHeader));
        }
        headers->arr[headers->count].key = gcArenaAllocate(headerKey.length + 1, alignof(char));
        headers->arr[headers->count].value = gcArenaAllocate(headerValue.length + 1, alignof(char));
        snprintf(headers->arr[headers->count].key, headerKey.length + 1, "%.*s", (int) headerKey.length, headerKey.ptr);
        snprintf(headers->arr[headers->count].value, headerValue.length + 1, "%.*s", (int) headerValue.length, headerValue.ptr);
        headers->count++;
    }
    stream->cursor += 2;
    return 0;
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
    if (str == NULL || n != 8 || strncmp(str, "HTTP/", 5) != 0)
    {
        return 0;
    }
#define IS_EQUAL(ver)                  \
    if (strncmp(str + 5, ver, 3) == 0) \
        return 1;
    IS_EQUAL("0.9");
    IS_EQUAL("1.0");
    IS_EQUAL("1.1");
    IS_EQUAL("2.0");
    IS_EQUAL("3.0");
#undef IS_EQUAL
    return 0;
}

#define DEFAULT_VERSION 11
int getVersionNumber(const char *str, int n)
{
    if (str == NULL)
    {
        return DEFAULT_VERSION;
    }
    if (n != 8 || strncmp(str, "HTTP/", 5) != 0)
    {
        return -1;
    }
#define IS_EQUAL(ver, num)             \
    if (strncmp(str + 5, ver, 3) == 0) \
        return num;
    IS_EQUAL("0.9", 9);
    IS_EQUAL("1.0", 10);
    IS_EQUAL("1.1", 11);
    IS_EQUAL("2.0", 20);
    IS_EQUAL("3.0", 30);
#undef IS_EQUAL
    return -1;
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

HttpHeaders emptyHeaders()
{
    return (HttpHeaders){NULL, 0};
}
