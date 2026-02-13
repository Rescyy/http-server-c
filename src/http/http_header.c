//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <alloc.h>
#include <http_header.h>
#include <string.h>
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

HttpHeader *findHeader(HttpHeaders *headers, const char *key)
{
    return (void*) findKeyValue((void *) headers->arr, headers->count, key);
}

HttpHeaders emptyHeaders()
{
    return (HttpHeaders){NULL, 0};
}
