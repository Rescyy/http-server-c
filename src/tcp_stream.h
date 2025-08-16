//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef TCP_STREAM_H
#define TCP_STREAM_H

#include "utils.h"
#include "errors.h"

#define TCP_STREAM_BUFFER_SIZE 1024

typedef struct TcpStream {
    int fd;
    int error;
    int cursor;
    int length;
    int capacity;
    char *buffer;
} TcpStream;

/* Creates a blocking TcpStream. */
TcpStream newTcpStream(int fd);
/* Frees allocated buffer in the tcp stream. */
void freeTcpStream(TcpStream *stream);
/* Waits for data to be available. */
void tcpStreamWait(TcpStream *stream);
/* Fills the internal buffer at least length. */
void tcpStreamFill(TcpStream *stream, int length);
/* Advances the cursor by size. Writes in the buffer. */
void tcpStreamRead(TcpStream *stream, void *buffer, int size);
/* Advances the cursor by size. Returns ptr. */
void *tcpStreamReadSlice(TcpStream *stream, int size);
/* Drains internal buffer until cursor. Performs memmove. */
void tcpStreamDrain(TcpStream *stream);
/* Reads and returns one byte at cursor, moves the cursor. */
char tcpStreamGetc(TcpStream *stream);
/* Reads and returns one byte at index. */
char tcpStreamIndex(TcpStream *stream, int index);
/* Reads until space character. */
string tcpStreamReadUntilSpace(TcpStream *stream, int maxLength);
/* Read until carriage return and new line. */
string tcpStreamReadUntilCRLF(TcpStream *stream, int maxLength, int ignoreLoneCRLF);
/* Read until string, returns the slice with length not including the substr. */
string tcpStreamReadUntilString(TcpStream *stream, int maxLength, const char *str, int size);
int tcpStreamHasError(TcpStream *stream);

#endif //TCP_STREAM_H
