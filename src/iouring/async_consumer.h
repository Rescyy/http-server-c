//
// Created by Rescyy on 10/31/2025.
//

#ifndef HTTPSERVERC_ASYNC_CONSUMER_H
#define HTTPSERVERC_ASYNC_CONSUMER_H

#include "common.h"

void consumeAsyncRequest(struct io_uring *ring);

#endif //HTTPSERVERC_ASYNC_CONSUMER_H