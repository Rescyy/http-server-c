//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_REQ_H
#define HTTP_REQ_H

#include "app_state.h"
#include "http_header.h"
#include "json.h"
#include "http_path.h"
#include "http_query.h"

typedef enum HttpMethod {
    GET,
    POST,
    PATCH,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    TRACE_,
    CONNECT,
    METHOD_UNKNOWN = -1,
} HttpMethod;

typedef struct HttpReq {
    HttpMethod method;
    HttpPath path;
    HttpQuery query;
    char *version;
    HttpHeaders headers;
    void *content;
    long contentLength;
    SessionState *appState;
    void *raw;
    size_t rawLength;
} HttpReq;

HttpReq newRequest();
int parseRequestStream(HttpReq *req, TcpStream *stream);
const char *methodToStr(HttpMethod method);
HttpMethod strnToMethod(const char *str, int n);
int reqEq(HttpReq obj1, HttpReq obj2);
void freeReq(HttpReq *req);
int isConnectionKeepAlive(HttpReq *req);
JObject httpReqToJObject(HttpReq *req);
void freeHttpReqJObject(JObject *req);

#endif //HTTP_REQ_H
