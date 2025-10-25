//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "../includes/tcp_stream.h"
#include "../includes/connection.h"
#include "../includes/logging.h"
#include "../includes/errors.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../includes/alloc.h"
#include <string.h>


#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define POLL_TIMEOUT (60 * 1000)

TcpStream *newTcpStream(TcpSocket *socket) {
    char *buffer = allocate(TCP_STREAM_BUFFER_SIZE);
    TcpStream *stream = allocate(sizeof(TcpStream));
    *stream = (TcpStream){
        .socket = socket,
        .error = 0,
        .cursor = 0,
        .length = 0,
        .capacity = TCP_STREAM_BUFFER_SIZE,
        .buffer = buffer
    };
    return stream;
}

void freeTcpStream(TcpStream *stream)
{
    debug("Freeing tcp stream");
    deallocate(stream->buffer);
    deallocate(stream);
}

#define MIN_FILL_SIZE 1024

void tcpStreamFill(TcpStream *stream, size_t length)
{
    if (stream->socket->closed) {
        stream->error = TCP_STREAM_CLOSED;
    }
    if (stream->length >= length)
    {
        return;
    }
    size_t newCapacity = stream->capacity;
    while (newCapacity < length)
    {
        newCapacity *= 2;
    }
    if (newCapacity > stream->capacity)
    {
        stream->buffer = reallocate(stream->buffer, newCapacity);
        stream->capacity = newCapacity;
    }
    while (stream->length < length)
    {
        /*
        In case the user wants to read only a few bytes,
        the stream will only read a max of MIN_FILL_SIZE, as to
        not fill it too much, and then possibly have to drain it.

        If the user needs more bytes than MIN_FILL_SIZE, then it will receive that
        amount of bytes directly.
        */
        const size_t maxReadSize = MAX(length - stream->length, MIN_FILL_SIZE);
        const size_t minReadSize = MIN(stream->capacity - stream->length, maxReadSize);
        ReadResult result = receive(stream->socket, stream->buffer + stream->length, minReadSize);

        switch (result.result) {
            case READ_OK:
                break;
            case READ_CLOSED:
                stream->error = TCP_STREAM_CLOSED;
                return;
            case READ_TIMEOUT:
                stream->error = TCP_STREAM_TIMEOUT;
                return;
            case READ_POLL_ERROR:
            case READ_RECV_ERROR:
                stream->error = TCP_STREAM_ERROR;
                return;
        }
        stream->length += result.received;
    }
}

void *tcpStreamReadSlice(TcpStream *stream, size_t size)
{
    tcpStreamFill(stream, stream->cursor + size);
    if (stream->error != 0)
    {
        return NULL;
    }
    void *slice = stream->buffer + stream->cursor;
    stream->cursor += size;
    return slice;
}

void tcpStreamDrain(TcpStream *stream)
{
    if (stream->length > stream->cursor) {
        memmove(stream->buffer, stream->buffer + stream->cursor, stream->length - stream->cursor);
    }
    stream->length -= stream->cursor;
    stream->cursor = 0;
    stream->error = 0;
}


string tcpStreamReadUntilSpace(TcpStream *stream, size_t maxLength)
{
    const size_t start = stream->cursor;
    size_t check = MIN(stream->length - stream->cursor, maxLength);
    for (; stream->cursor <= start + check; stream->cursor++) {
        if (stream->buffer[stream->cursor] == ' ') {
            const string result = (string) {
                .ptr = stream->buffer + start,
                .length = stream->cursor - start,
            };
            stream->cursor++;
            return result;
        }
    }
    for (;;) {
        if (stream->cursor > start + maxLength)
        {
            return (string){.ptr = NULL, .length = ENTITY_TOO_LARGE_ERROR};
        }
        tcpStreamFill(stream, maxLength);
        if (stream->error < 0) {
            return (string){
                .ptr = NULL, .length = stream->error
            };
        }
        check = MIN(stream->length - stream->cursor, maxLength);
        for (; stream->cursor < start + check; stream->cursor++) {
            if (stream->buffer[stream->cursor] == ' ') {
                const string result = (string) {
                    .ptr = stream->buffer + start,
                    .length = stream->cursor - start,
                };
                stream->cursor++;
                return result;
            }
        }
    }
}

string tcpStreamReadUntilCRLF(TcpStream *stream, size_t maxLength, int ignoreLoneCRLF)
{
    int start = stream->cursor;

    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + 2);
        if (stream->error < 0)
        {
            return (string){.ptr = NULL, .length = stream->error};
        }

        if (stream->buffer[stream->cursor] == '\r' && stream->buffer[stream->cursor + 1] == '\n')
        {
            stream->cursor += 2;
            return (string){
                .ptr = stream->buffer + start,
                .length = stream->cursor - start - 2,
            };
        }
        if (!ignoreLoneCRLF && (stream->buffer[stream->cursor] == '\n' || stream->buffer[stream->cursor] == '\r')) {
            return (string){.ptr = NULL, .length = BAD_REQUEST_ERROR};
        }
        stream->cursor++;
        if (stream->cursor - start > maxLength)
        {
            return (string){.ptr = NULL, .length = ENTITY_TOO_LARGE_ERROR};
        }
    }
}

string tcpStreamReadUntilString(TcpStream *stream, size_t maxLength, const char *subStr, size_t size)
{
    assert(size > 0);
    int start = stream->cursor;
    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + size);
        if (stream->error < 0)
        {
            return (string){.ptr = NULL, .length = stream->error};
        }
        if (strncmp(stream->buffer + stream->cursor, subStr, size) == 0)
        {
            stream->cursor += size;
            return (string){
                .ptr = stream->buffer + start,
                .length = stream->cursor - start - size,
            };
        }
        stream->cursor++;
        if (stream->cursor - start > maxLength + size)
        {
            return (string){.ptr = NULL, .length = ENTITY_TOO_LARGE_ERROR};
        }
    }
}