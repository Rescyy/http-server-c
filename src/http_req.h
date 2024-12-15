#ifndef HTTP_REQ_H
#define HTTP_REQ_H

#include "http.h"

typedef enum HttpMethod {
    GET,
    POST,
    METHOD_UNKNOWN = -1,
} HttpMethod;

typedef struct HttpPath {
    char **elements;
    int elCount;
} HttpPath;

typedef struct HttpReq {
    HttpMethod method;
    HttpPath path;
    char *version;
    HttpHeaders headers;
    void *content;
    int contentWritten;
    int contentLength;
} HttpReq;

int parseRequest(HttpReq *reqObj, void *reqBytes, int size);
const char *methodToStr(HttpMethod method);
HttpMethod strnToMethod(const char *str, int n);
int parsePath(HttpPath *path, const char *str, int n);
void freePath(HttpPath *path);
int pathEq(HttpPath obj1, HttpPath obj2);
int pathMatches(HttpPath endpointPath, HttpPath reqPath);
int pathToStr(char *str, int size, HttpPath path);
int reqEq(HttpReq obj1, HttpReq obj2);
int reqIsComplete(HttpReq req);
void freeReq(HttpReq *req);
void addAdditionalRequestContent(HttpReq *reqObj, void *additionalContent, int additionalBytes);

#endif