//
// Created by Rescyy on 2/9/2026.
//

#ifndef HTTPSERVERC_HTTP_QUERY_H
#define HTTPSERVERC_HTTP_QUERY_H
#include <utils.h>
#include <tcp_stream.h>

typedef struct {
    string key;
    string value;
} HttpQueryParameter;

typedef struct {
    HttpQueryParameter *parameters;
    size_t count;
} HttpQuery;

int parseQuery(HttpQuery *query, const char *str, size_t len);
HttpQueryParameter *findQueryParameter(HttpQuery *headers, const char *key);

#endif //HTTPSERVERC_HTTP_QUERY_H