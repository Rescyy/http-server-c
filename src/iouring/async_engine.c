//
// Created by Rescyy on 11/1/2025.
//

#include "async_engine.h"
#include "common.h"

struct io_uring initializeIouRing() {
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);
    return ring;
}