//
// Created by Rescyy on 2/9/2026.
//

#ifndef HTTPSERVERC_HTTP_QUERY_H
#define HTTPSERVERC_HTTP_QUERY_H
#include <utils.h>

typedef struct {
    string key;
    string value;
} HttpQueryParameter;

typedef struct {
    HttpQueryParameter *parameters;
    size_t count;
} HttpQuery;

#endif //HTTPSERVERC_HTTP_QUERY_H