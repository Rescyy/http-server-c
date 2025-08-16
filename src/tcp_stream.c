//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "tcp_stream.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "alloc.h"
#include <string.h>
#include <sys/socket.h>
#include <poll.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

TcpStream newTcpStream(int fd)
{
    char *buffer = allocate(TCP_STREAM_BUFFER_SIZE);
    if (buffer == NULL)
    {
        return (TcpStream){
            .fd = -1,
            .error = errno,
            .cursor = 0,
            .length = 0,
            .capacity = 0,
            .buffer = NULL};
    }
    return (TcpStream){
        .fd = fd,
        .error = 0,
        .cursor = 0,
        .length = 0,
        .capacity = TCP_STREAM_BUFFER_SIZE,
        .buffer = buffer};
}

void freeTcpStream(TcpStream *stream)
{
    deallocate(stream->buffer);
}

void tcpStreamWait(TcpStream *stream)
{
    struct pollfd fds = {
        .fd = stream->fd,
        .events = POLLIN | POLLPRI,
    };

    int result = poll(&fds, (nfds_t)1, -1);


    if (result < 0 || fds.revents & (POLLERR | POLLNVAL))
    {
        stream->error = errno;
        return;
    }

    if (fds.revents & POLLHUP)
    {
        stream->error = TCP_STREAM_CLOSED;
        return;
    }

    if (fds.revents & (POLLIN | POLLPRI))
    {
        return;
    }
}

int tcpStreamWaitTimeout(TcpStream *stream, int timeout)
{
    struct pollfd fds = {
        .fd = stream->fd,
        .events = POLLIN | POLLPRI,
    };

    int result = poll(&fds, (nfds_t)1, timeout);
    return result;
}

#define MIN_FILL_SIZE 1024

void tcpStreamFill(TcpStream *stream, int length)
{
    if (stream->length >= length)
    {
        return;
    }
    int newCapacity = stream->capacity;
    while (newCapacity < length)
    {
        newCapacity *= 2;
    }
    if (newCapacity > stream->capacity)
    {
        stream->buffer = reallocate(stream->buffer, newCapacity);
        if (stream->buffer == NULL)
        {
            stream->error = errno;
            return;
        }
        stream->capacity = newCapacity;
    }
    while (stream->length < length)
    {
        tcpStreamWait(stream);
        if (stream->error != 0)
        {
            return;
        }

        /*
        In case the user wants to read only a few bytes,
        the stream will only read a max of MIN_FILL_SIZE, as to
        not fill it too much, and then possibly have to drain it.

        If the user needs more bytes than MIN_FILL_SIZE, then it will receive that
        amount of bytes directly.
        */
        int maxReadSize = MAX(length - stream->length, MIN_FILL_SIZE);
        int minReadSize = MIN(stream->capacity - stream->length, maxReadSize);
        int result = recv(
            stream->fd,
            stream->buffer + stream->length,
            minReadSize,
            0);

        if (result < 0)
        {
            stream->error = errno;
            return;
        }
        stream->length += result;
    }
}

void tcpStreamRead(TcpStream *stream, void *buffer, int size)
{
    tcpStreamFill(stream, stream->cursor + size);
    if (stream->error != 0)
    {
        return;
    }
    int start = stream->cursor;
    stream->cursor += size;
    memcpy(buffer, stream->buffer + start, size);
}

void *tcpStreamReadSlice(TcpStream *stream, int size)
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
    memmove(stream->buffer, stream->buffer + stream->cursor, stream->length - stream->cursor);
    stream->length -= stream->cursor;
    stream->cursor = 0;
    stream->error = 0;
}

char tcpStreamGetc(TcpStream *stream)
{
    tcpStreamFill(stream, stream->cursor + 1);
    return stream->buffer[stream->cursor++];
}

char tcpStreamIndex(TcpStream *stream, int index)
{
    tcpStreamFill(stream, index + 1);
    return stream->buffer[index];
}

string tcpStreamReadUntilSpace(TcpStream *stream, int maxLength)
{
    int start = stream->cursor;
    while (tcpStreamGetc(stream) != ' ')
    {
        if (stream->error == TCP_STREAM_CLOSED)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_CLOSED};
        }
        if (stream->error != 0)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_ERROR};
        }
        if (stream->cursor - start > maxLength)
        {
            return (string){.ptr = NULL, .length = ENTITY_TOO_LARGE_ERROR};
        }
    }

    return (string){
        .ptr = stream->buffer + start,
        .length = stream->cursor - start - 1,
    };
}

string tcpStreamReadUntilCRLF(TcpStream *stream, int maxLength, int ignoreLoneCRLF)
{
    int start = stream->cursor;

    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + 2);
        if (stream->error == TCP_STREAM_CLOSED)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_CLOSED};
        }
        if (stream->error != 0)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_ERROR};
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

string tcpStreamReadUntilString(TcpStream *stream, int maxLength, const char *subStr, int size)
{
    assert(size > 0);
    int start = stream->cursor;
    for (;;)
    {
        tcpStreamFill(stream, stream->cursor + size);
        if (stream->error == TCP_STREAM_CLOSED)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_CLOSED};
        }
        if (stream->error != 0)
        {
            return (string){.ptr = NULL, .length = TCP_STREAM_ERROR};
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

int tcpStreamHasError(TcpStream *stream)
{
    int error = stream->error;
    stream->error = 0;
    return error != 0;
}