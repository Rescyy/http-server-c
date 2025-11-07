//
// Created by Rescyy on 10/30/2025.
//

#include "async_producer.h"
#include "common.h"

#include <liburing.h>
#include <alloc.h>

void acceptMultishotAsync(struct io_uring *ring, AcceptMultishotAsyncRequest *request) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_multishot_accept(sqe, request->fd, NULL, NULL, 0);
    io_uring_sqe_set_data(sqe, request);
    io_uring_submit(ring);
}

void receiveAsync(struct io_uring *ring, ReadAsyncRequest *request) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe, request->fd, request->buffer, request->len, 0);
    io_uring_sqe_set_data(sqe, request);
    io_uring_submit(ring);
}

void transmitAsync(struct io_uring *ring, WriteAsyncRequest *request) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, request->fd, request->buffer, request->len, 0);
    io_uring_sqe_set_data(sqe, request);
    io_uring_submit(ring);
}

