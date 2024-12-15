#ifndef HTTP_H
#define HTTP_H

typedef struct HttpHeader {
    char *key;
    char *value;
} HttpHeader;

typedef struct HttpHeaders {
    HttpHeader *arr;
    int count;
} HttpHeaders;

int parseHeaders(HttpHeaders *headers, const char *str, int size);
void freeHeaders(HttpHeaders *headers);
int isVersionValid(const char *str, int n);
int headerEq(HttpHeader obj1, HttpHeader obj2);
int headersEq(HttpHeaders obj1, HttpHeaders obj2);
HttpHeader *findHeader(HttpHeaders headers, const char *key);

#endif