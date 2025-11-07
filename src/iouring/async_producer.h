//
// Created by Rescyy on 10/30/2025.
//

#ifndef HTTPSERVERC_IOURING_H
#define HTTPSERVERC_IOURING_H

#include "common.h"
#include <stdlib.h>

typedef struct {
    AsyncCallback callback;
    int fd;
    void *userData;
} AcceptMultishotAsyncRequest;

void acceptMultishotAsync(struct io_uring *ring, AcceptMultishotAsyncRequest *request);

typedef struct {
    AsyncCallback callback;
    int fd;
    void *buffer;
    size_t len;
    void *userData;
} ReadAsyncRequest;

void receiveAsync(struct io_uring *ring, ReadAsyncRequest *request);

typedef struct {
    AsyncCallback callback;
    int fd;
    void *buffer;
    size_t len;
    void *userData;
} WriteAsyncRequest;

void transmitAsync(struct io_uring *ring, WriteAsyncRequest *request);

#endif //HTTPSERVERC_IOURING_H