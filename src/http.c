//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "http.h"
#include <string.h>
#include <stdio.h>
#include "alloc.h"
#include "utils.h"

int parseHeaders(HttpHeaders *headers, const char *str, int size)
{
    int offset, prevOffset = 0, headerCap = 1;
    headers->arr = allocate(sizeof(HttpHeader));
    headers->count = 0;

    while (strncmp(str + prevOffset, "\r\n", 2) != 0)
    {
        if (headers->count >= headerCap)
        {
            headerCap *= 2;
            headers->arr = reallocate(headers->arr, headerCap * sizeof(HttpHeader));
        }
        offset = strnindex(str + prevOffset, size - prevOffset, ": ");
        if (offset == -1 ||
            strnindex(str + prevOffset, offset, "\r") != -1 ||
            strnindex(str + prevOffset, offset, "\n") != -1)
        {
            freeHeaders(headers);
            return -1;
        }
        char *key = allocate(offset + 1);
        headers->arr[headers->count].key = key;
        snprintf(key, offset + 1, "%s", str + prevOffset);
        prevOffset += offset + 2;
        if (str[prevOffset] == ' ')
        {
            freeHeaders(headers);
            deallocate(key);
            return -1;
        }

        offset = strnindex(str + prevOffset, size - prevOffset, "\r\n");
        if (offset == -1 ||
            strnindex(str + prevOffset, offset, "\r") != -1 ||
            strnindex(str + prevOffset, offset, "\n") != -1)
        {
            deallocate(key);
            freeHeaders(headers);
            return -1;
        }
        char *value = allocate(offset + 1);
        headers->arr[headers->count].value = value;
        snprintf(value, offset + 1, "%s", str + prevOffset);
        prevOffset += offset + 2;

        headers->count++;
    }

    return prevOffset + 2;
}

#define INITIAL_HEADER_CAP 4
#define MAX_HEADER_SIZE 8192

int parseHeadersStream(HttpHeaders *headers, TcpStream *stream)
{
    int headerCap = INITIAL_HEADER_CAP;
    headers->arr = allocate(sizeof(HttpHeader) * headerCap);
    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + 2);
        if (stream->error != 0)
        {
            freeHeaders(headers);
            return stream->error;
        }
        if(strncmp(stream->buffer + stream->cursor, "\r\n", 2) == 0) {
            break;
        }
        string headerKey = tcpStreamReadUntilString(stream, MAX_HEADER_SIZE, ": ", 2);
        if (headerKey.length < 0)
        {
            freeHeaders(headers);
            return headerKey.length;
        }
        string headerValue = tcpStreamReadUntilCRLF(stream, MAX_HEADER_SIZE, 0);
        if (headerKey.length < 0)
        {
            freeHeaders(headers);
            return headerValue.length;
        }
        if (headers->count >= headerCap)
        {
            headerCap *= 2;
            headers->arr = reallocate(headers->arr, headerCap * sizeof(HttpHeader));
        }
        headers->arr[headers->count].key = allocate(headerKey.length + 1);
        headers->arr[headers->count].value = allocate(headerValue.length + 1);
        snprintf(headers->arr[headers->count].key, headerKey.length + 1, "%s", headerKey.ptr);
        snprintf(headers->arr[headers->count].value, headerValue.length + 1, "%s", headerValue.ptr);
        headers->count++;
    }
    stream->cursor += 2;
    return 0;
}

void freeHeaders(HttpHeaders *headers)
{
    if (headers->arr == NULL) {
        return;
    }
    for (int i = 0; i < headers->count; i++)
    {
        deallocate(headers->arr[i].key);
        deallocate(headers->arr[i].value);
    }
    deallocate(headers->arr);
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
