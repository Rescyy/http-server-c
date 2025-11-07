//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <http.h>
#include <string.h>
#include <alloc.h>
#include <utils.h>

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
        headers->arr[headers->count].key = copyString(headerKey);
        headers->arr[headers->count].value = copyString(headerValue);
        headers->count++;
    }
    stream->cursor += 2;
    return 0;
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

HttpHeader *findHeader(HttpHeaders *headers, const char *key)
{
    for (int i = 0; i < headers->count; i++)
    {
        if (strcasecmp(headers->arr[i].key.ptr, key) == 0)
        {
            return &headers->arr[i];
        }
    }
    return NULL;
}

HttpHeaders emptyHeaders()
{
    return (HttpHeaders){NULL, 0};
}
