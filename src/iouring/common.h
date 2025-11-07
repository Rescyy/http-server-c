//
// Created by Rescyy on 10/31/2025.
//

#ifndef HTTPSERVERC_COMMON_H
#define HTTPSERVERC_COMMON_H

#include <liburing.h>

#define QUEUE_DEPTH 1024

typedef void (*AsyncCallback)(int, void*);

#endif //HTTPSERVERC_COMMON_H