//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include "utils.h"
#include "connection.h"

#define TCP_STREAM_BUFFER_SIZE 1024

typedef struct TcpStream {
    TcpSocket *socket;
    int error;
    size_t cursor;
    size_t length;
    size_t capacity;
    char *buffer;
} TcpStream;

/* Creates a blocking TcpStream. */
TcpStream *newTcpStream(TcpSocket *socket);
/* Frees allocated buffer in the tcp stream. */
void freeTcpStream(TcpStream *stream);
/* Waits for data to be available. */
void tcpStreamWait(TcpStream *stream);
/* Fills the internal buffer at least length. */
void tcpStreamFill(TcpStream *stream, size_t length);
/* Advances the cursor by size. Returns ptr. */
void *tcpStreamReadSlice(TcpStream *stream, size_t size);
/* Drains internal buffer until cursor. Performs memmove. */
void tcpStreamDrain(TcpStream *stream);
/* Reads until space character. */
string tcpStreamReadUntilSpace(TcpStream *stream, size_t maxLength);
/* Read until carriage return and new line. */
string tcpStreamReadUntilCRLF(TcpStream *stream, size_t maxLength, int ignoreLoneCRLF);
/* Read until string, returns the slice with length not including the substr. */
string tcpStreamReadUntilString(TcpStream *stream, size_t maxLength, const char *str, size_t size);
/* Read until find any character from anyChar cString */
string tcpStreamReadUntilAny(TcpStream *stream, size_t maxLength, const char *anyChar);

#endif //TCP_STREAM_H
