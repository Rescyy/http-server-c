//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "../includes/alloc.h"
#include "../includes/errors.h"
#include "../includes/http_req.h"
#include "../includes/utils.h"
#include "../includes/tcp_stream.h"
#include "../includes/errors.h"
#include "../includes/logging.h"

long findContentLength(HttpHeaders *headers);
static int parsePath(HttpPath *path, const char *str, int n);

HttpReq newRequest()
{
    return (HttpReq){
        .method = METHOD_UNKNOWN,
        .path = {NULL, 0},
        .version = NULL,
        .headers = emptyHeaders(),
        .content = NULL,
        .contentLength = 0,
        .appState = NULL,
    };
}

/* Parse HTTP request, returns the amount of bytes needed additionally read from the TCP socket based on the Content-Length header */
int parseRequestStream(HttpReq *req, TcpStream *stream)
{
    /* Parse method */
    {
        debug("Parsing Method");
        string method = tcpStreamReadUntilSpace(stream, 7);
        if (method.length < 0)
        {
            if (method.length != TCP_STREAM_CLOSED && method.length != TCP_STREAM_TIMEOUT) {
                error("Error Parsing Method: %s", errToStr(method.length));
            }
            return method.length;
        }
        req->method = strnToMethod(method.ptr, method.length);

        if (req->method == METHOD_UNKNOWN)
        {
            error("Error parsing Method UNKNOWN");
            return UNKNOWN_METHOD;
        }
    }

    /* Parse path */
    {
        debug("Parsing Path");
        string path = tcpStreamReadUntilSpace(stream, 1024);
        if (path.length < 0)
        {
            if (path.length != TCP_STREAM_CLOSED && path.length != TCP_STREAM_TIMEOUT) {
                error("Error Parsing Path: %s", errToStr(path.length));
            }
            return path.length;
        }
        if (parsePath(&req->path, path.ptr, path.length) == -1)
        {
            return BAD_REQUEST_ERROR;
        }
    }

    /* Parse version */
    {
        debug("Parsing Version");
        string version = tcpStreamReadUntilCRLF(stream, 8, 0);
        if (version.length < 0 && version.length != ENTITY_TOO_LARGE_ERROR)
        {
            if (version.length != TCP_STREAM_CLOSED && version.length != TCP_STREAM_TIMEOUT) {
                error("Error Parsing Version: %s\n", errToStr(version.length));
            }
            return version.length;
        }
        if (!isVersionValid(version.ptr, version.length) || version.length == ENTITY_TOO_LARGE_ERROR)
        {
            error("Error Parsing Version TOO LARGE\n");
            return UNKNOWN_VERSION;
        }
        req->version = gcArenaAllocate(version.length + 1, alignof(char));
        strncpy(req->version, version.ptr, version.length);
    }

    /* Parse headers */
    {
        debug("Parsing Headers");
        int result = parseHeadersStream(&req->headers, stream);
        if (result < 0)
        {
            if (result != TCP_STREAM_CLOSED && result != TCP_STREAM_TIMEOUT) {
                error("Error Parsing Headers %s\n", errToStr(result));
            }
            return result;
        }
    }

    /* Fetch content */
    {
        debug("Parsing Content");
        req->contentLength = findContentLength(&req->headers);
        if (req->contentLength > 0)
        {
            req->content = tcpStreamReadSlice(stream, req->contentLength);
            if (stream->error < 0)
            {
                if (stream->error != TCP_STREAM_CLOSED && stream->error != TCP_STREAM_TIMEOUT) {
                    error("Error Fetching Content: %s\n", errToStr(stream->error));
                }
                return stream->error;
            }
        }
        else if (req->contentLength == 0)
        {
            req->content = NULL;
        }
        else
        {
            return BAD_REQUEST_ERROR;
        }
    }

    req->raw = stream->buffer;
    req->rawLength = stream->cursor;

    return 0;
}

long findContentLength(HttpHeaders *headers)
{
    HttpHeader *contentLengthHeader = findHeader(headers, "Content-Length");
    if (contentLengthHeader == NULL) {
        return 0;
    }
    long contentLength = strtol(contentLengthHeader->value.ptr, NULL, 10);
    if (errno == ERANGE || contentLength < 0) {
        return -1;
    }

    return contentLength;
}

const char *methodToStr(HttpMethod method)
{
    static const char get[] = "GET";
    static const char post[] = "POST";
    static const char patch[] = "PATCH";
    static const char put[] = "PUT";
    static const char delete[] = "DELETE";
    static const char head[] = "HEAD";
    static const char options[] = "OPTIONS";
    static const char trace[] = "TRACE";
    static const char connect[] = "CONNECT";
    static const char unknown[] = "UNK";

    switch (method)
    {
        case GET:
            return get;
        case POST:
            return post;
        case PATCH:
            return patch;
        case PUT:
            return put;
        case DELETE:
            return delete;
        case HEAD:
            return head;
        case OPTIONS:
            return options;
        case TRACE:
            return trace;
        case CONNECT:
            return connect;
        default:
            return unknown;
    }
}

HttpMethod strnToMethod(const char *str, int n)
{
    if (str == NULL)
        return METHOD_UNKNOWN;
#define IS_METHOD(method)                          \
    if (strncmp(methodToStr(method), str, n) == 0) \
    return (method)
    IS_METHOD(GET);
    IS_METHOD(POST);
    IS_METHOD(PATCH);
    IS_METHOD(PUT);
    IS_METHOD(DELETE);
    IS_METHOD(HEAD);
    IS_METHOD(OPTIONS);
    IS_METHOD(TRACE);
    IS_METHOD(CONNECT);
    return METHOD_UNKNOWN;
#undef IS_METHOD
}

static int parsePath(HttpPath *path, const char *str, int n)
{
    int offset, prevOffset = 0;
    path->elCount = 0;
    path->elements = NULL;
    path->raw = gcArenaAllocate(n + 1, alignof(char));
    snprintf(path->raw, n + 1, "%.*s", n, str);
    path->raw[n] = '\0';
    char *element;
    int slashes = 0;
    for (int i = 0; i < n; i++) {
        slashes += str[i] == '/';
    }
    if (str[prevOffset++] != '/') {
        return -1;
    }
    if (n == 1) {
        path->elements = NULL;
        return 0;
    }

    path->elements = gcArenaAllocate(sizeof(char *) * slashes, alignof(char *));

    while ((offset = strnindex(str + prevOffset, n - prevOffset, "/")) != -1)
    {
        element = gcArenaAllocate(offset + 1, alignof(char));
        path->elements[path->elCount++] = element;
        snprintf(element, offset + 1, "%.*s", offset, str + prevOffset);

        prevOffset += offset + 1;
    }

    int elCap = n - prevOffset + 1;

    if (elCap > 1)
    {
        element = gcArenaAllocate(elCap, alignof(char));
        path->elements[path->elCount++] = element;
        snprintf(element, elCap, "%.*s", elCap - 1, str + prevOffset);
    }

    return 0;
}

/*
    Matches path to endpoint path of form
    /object/<str>/<int> to match /object/hi/123
*/
int pathMatches(HttpPath endpointPath, HttpPath reqPath)
{
    if (endpointPath.elCount != reqPath.elCount)
    {
        return 0;
    }
    for (int i = 0; i < endpointPath.elCount; i++)
    {
        if (strcmp(endpointPath.elements[i], "<str>") == 0)
        {
            continue;
        }
        else if (strcmp(endpointPath.elements[i], "<int>") == 0)
        {
            long result = strtol(reqPath.elements[i], NULL, 10);
            if (result == 0 && strcmp(reqPath.elements[i], "0") != 0)
            {
                return 0;
            }
        }
        else if (strcmp(endpointPath.elements[i], reqPath.elements[i]) != 0)
        {
            return 0;
        }
    }
    return 1;
}

/*miau*/

int isConnectionKeepAlive(HttpReq *req)
{
    HttpHeader *header = findHeader(&req->headers, "Connection");
    if (header == NULL)
    {
        if (getVersionNumber(req->version, 8) >= 11)
        {
            return 1;
        }
        return 0;
    }
    return strcasecmp(header->value.ptr, "keep-alive") == 0;
}

JObject httpReqToJObject(HttpReq *req) {
    static const char methodKey[] = "method";
    static const char pathKey[] = "path";
    static const char versionKey[] = "version";
    static const char headersKey[] = "headers";
    static const char contentKey[] = "content";

    char *path = malloc(1024);
    strncpy(path, req->path.raw, 1024);
    JObject headersObj = {
        .properties = gcArenaAllocate(sizeof(JProperty) * req->headers.count, alignof(JProperty)),
        .count = req->headers.count
    };
    HttpHeader *headers = req->headers.arr;
    for (int i = 0; i < req->headers.count; i++) {
        headersObj.properties[i] = _JProperty(headers[i].key.ptr, toJToken_cstring(headers[i].value.ptr));
    }

    JToken contentToken;
    if (req->content == NULL) {
        contentToken = _JNull();
    } else {
        contentToken = toJToken_cstring(req->content);
    }

    JObject reqObject = _JObject(
        _JProperty(methodKey, toJToken_cstring(methodToStr(req->method))),
        _JProperty(pathKey, toJToken_cstring(path)),
        _JProperty(versionKey, toJToken_cstring(req->version)),
        _JProperty(headersKey, toJToken_JObject(headersObj)),
        _JProperty(contentKey, contentToken)
    );

    return reqObject;
}