//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_REQ_H
#define HTTP_REQ_H

#include "app_state.h"
#include "http.h"
#include "json.h"

typedef enum HttpMethod {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    TRACE,
    CONNECT,
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
    int contentLength;
    SessionState *appState;
} HttpReq;

HttpReq newRequest();
int parseRequestStream(HttpReq *reqObj, TcpStream *stream);
const char *methodToStr(HttpMethod method);
HttpMethod strnToMethod(const char *str, int n);
int parsePath(HttpPath *path, const char *str, int n);
void freePath(HttpPath *path);
int pathEq(HttpPath obj1, HttpPath obj2);
int pathMatches(HttpPath endpointPath, HttpPath reqPath);
int pathToStr(char *str, int size, HttpPath path);
int reqEq(HttpReq obj1, HttpReq obj2);
void freeReq(HttpReq *req);
int isConnectionKeepAlive(HttpReq *req);
JObject httpReqToJObject(HttpReq *req);
void freeHttpReqJObject(JObject *req);

#endif //HTTP_REQ_H
