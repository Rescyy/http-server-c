//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_H
#define HTTP_H

#include "tcp_stream.h"
#include "utils.h"

typedef struct HttpHeader {
    string key;
    string value;
} HttpHeader;

typedef struct HttpHeaders {
    HttpHeader *arr;
    int count;
} HttpHeaders;

int parseHeadersStream(HttpHeaders *headers, TcpStream *stream);
int isVersionValid(const char *str, int n);
int getVersionNumber(const char *str, int n);
HttpHeader *findHeader(HttpHeaders *headers, const char *key);
HttpHeaders emptyHeaders();

#endif //HTTP_H
