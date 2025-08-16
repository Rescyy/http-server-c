//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_H
#define HTTP_H

#include "tcp_stream.h"

typedef struct HttpHeader {
    char *key;
    char *value;
} HttpHeader;

typedef struct HttpHeaders {
    HttpHeader *arr;
    int count;
} HttpHeaders;

int parseHeaders(HttpHeaders *headers, const char *str, int size);
int parseHeadersStream(HttpHeaders *headers, TcpStream *stream);
void freeHeaders(HttpHeaders *headers);
int isVersionValid(const char *str, int n);
int getVersionNumber(const char *str, int n);
int headerEq(HttpHeader obj1, HttpHeader obj2);
int headersEq(HttpHeaders obj1, HttpHeaders obj2);
HttpHeader *findHeader(HttpHeaders headers, const char *key);
HttpHeaders emptyHeaders();

#endif //HTTP_H
