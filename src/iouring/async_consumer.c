//
// Created by Rescyy on 10/31/2025.
//

#include "async_consumer.h"
#include <alloc.h>

void consumeAsyncRequest(struct io_uring *ring) {
    struct io_uring_cqe *cqe;
    int result = io_uring_wait_cqe(ring, &cqe);
    AsyncCallback callback = (AsyncCallback) cqe->user_data;
    callback(result, cqe->user_data);
    deallocate(callback);
}