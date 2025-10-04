//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "alloc.h"
#include "errors.h"
#include "http_req.h"
#include "utils.h"
#include "tcp_stream.h"
#include "errors.h"
#include "logging.h"

int findContentLength(HttpHeaders headers);

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
    SessionState *state = req->appState;
    int returnValue = 0;
    /* Parse method */
    {
        string method = tcpStreamReadUntilSpace(stream, 7);
        if (method.length < 0)
        {
            returnValue = method.length;
            if (method.length != TCP_STREAM_CLOSED) {
                error("Error Parsing Method: %s", errToStr(returnValue));
            }
            goto _ErrorReturn;
        }
        req->method = strnToMethod(method.ptr, method.length);

        if (req->method == METHOD_UNKNOWN)
        {
            returnValue = UNKNOWN_METHOD;
            error("Error parsing Method UNKNOWN");
            goto _ErrorReturn;
        }
    }

    /* Parse path */
    {
        string path = tcpStreamReadUntilSpace(stream, 1024);
        if (path.length < 0)
        {
            returnValue = path.length;
            if (path.length != TCP_STREAM_CLOSED) {
                error("Error Parsing Path: %s", errToStr(returnValue));
            }
            goto _ErrorReturn;
        }
        if (parsePath(&req->path, path.ptr, path.length) == -1)
        {
            returnValue = BAD_REQUEST_ERROR;
            goto _ErrorReturn;
        }
    }

    /* Parse version */
    {
        string version = tcpStreamReadUntilCRLF(stream, 8, 0);
        if (version.length < 0 && version.length != ENTITY_TOO_LARGE_ERROR)
        {
            returnValue = version.length;
            if (version.length != TCP_STREAM_CLOSED) {
                error("Error Parsing Version: %s\n", errToStr(returnValue));
            }
            goto _ErrorReturn;
        }
        if (!isVersionValid(version.ptr, version.length) || version.length == ENTITY_TOO_LARGE_ERROR)
        {
            returnValue = UNKNOWN_VERSION;
            error("Error Parsing Version TOO LARGE\n");
            goto _ErrorReturn;
        }
        req->version = allocate(version.length + 1);
        strncpy(req->version, version.ptr, version.length);
    }

    /* Parse headers */
    {
        int result = parseHeadersStream(&req->headers, stream);
        if (result < 0)
        {
            returnValue = result;
            if (result != TCP_STREAM_CLOSED) {
                error("Error Parsing Headers %s\n", errToStr(result));
            }
            goto _ErrorReturn;
        }
    }

    /* Fetch content */
    {
        req->contentLength = findContentLength(req->headers);
        if (req->contentLength > 0)
        {
            req->content = tcpStreamReadSlice(stream, req->contentLength);
            if (stream->error < 0)
            {
                returnValue = stream->error;
                if (stream->error != TCP_STREAM_CLOSED) {
                    error("Error Fetching Content: %s\n", errToStr(stream->error));
                }
                goto _ErrorReturn;
            }
        }
        else
        {
            req->content = NULL;
        }
    }

    req->raw = stream->buffer;
    req->rawLength = stream->cursor;

    return 0;
_ErrorReturn:
    req->content = NULL;
    freeHeaders(&req->headers);
    deallocate(req->version);
    freePath(&req->path);
    return returnValue;
}

int findContentLength(HttpHeaders headers)
{
    for (int i = 0; i < headers.count; i++)
    {
        if (strcasecmp(headers.arr[i].key, "Content-Length") == 0)
        {
            int contentLength = atoi(headers.arr[i].value);
            return contentLength;
        }
    }

    return 0;
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

int parsePath(HttpPath *path, const char *str, int n)
{
    int offset, prevOffset = 0, pathCap = 1;
    path->elCount = 0;
    path->elements = NULL;
    path->raw = allocate(n + 1);
    snprintf(path->raw, n + 1, "%s", str);
    path->raw[n] = '\0';
    char *element;
    if (str[prevOffset++] != '/')
        return -1;

    path->elements = allocate(sizeof(char *));

    while ((offset = strnindex(str + prevOffset, n - prevOffset, "/")) != -1)
    {
        element = allocate(offset + 1);

        if (path->elCount >= pathCap)
        {
            pathCap *= 2;
            path->elements = reallocate(path->elements, sizeof(char *) * pathCap);
        }

        path->elements[path->elCount++] = element;
        snprintf(element, offset + 1, "%s", str + prevOffset);

        prevOffset += offset + 1;
    }

    int elCap = n - prevOffset + 1;

    if (elCap > 1)
    {
        element = allocate(elCap);
        if (path->elCount == pathCap)
        {
            pathCap++;
            path->elements = reallocate(path->elements, sizeof(char *) * pathCap);
        }
        path->elements[path->elCount++] = element;
        snprintf(element, elCap, "%s", str + prevOffset);
    }

    if (path->elCount == 0)
    {
        deallocate(path->elements);
        path->elements = NULL;
    }
    else if (path->elCount < pathCap)
    {
        path->elements = reallocate(path->elements, sizeof(char *) * path->elCount);
    }

    return 0;
}

void freePath(HttpPath *path)
{
    if (path->elements == NULL) {
        return;
    }
    for (int i = 0; i < path->elCount; i++)
    {
        deallocate(path->elements[i]);
    }
    deallocate(path->elements);
    deallocate(path->raw);
    path->elements = NULL;
}

int pathEq(HttpPath obj1, HttpPath obj2)
{
    if (obj1.elCount != obj2.elCount)
    {
        return 0;
    }

    for (int i = 0; i < obj1.elCount; i++)
    {
        if (strcmp(obj1.elements[i], obj2.elements[i]) != 0)
        {
            return 0;
        }
    }

    return 1;
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

int reqEq(HttpReq obj1, HttpReq obj2)
{ /*miau*/
    if (obj1.method != obj2.method)
    {
        return 0;
    }

    if (!pathEq(obj1.path, obj2.path))
    {
        return 0;
    }

    if (strcmp(obj1.version, obj2.version) != 0)
    {
        return 0;
    }

    if (!headersEq(obj1.headers, obj2.headers))
    {
        return 0;
    }

    if (obj1.contentLength != obj2.contentLength)
    {
        return 0;
    }

    if (obj1.contentLength == 0 && obj2.contentLength == 0)
    {
        return 1;
    }

    if (obj1.content == NULL || obj2.content == NULL || memcmp(obj1.content, obj2.content, obj1.contentLength) != 0)
    {
        return 0;
    }

    return 1;
}

void freeReq(HttpReq *req)
{
    freePath(&req->path);
    freeHeaders(&req->headers);
    deallocate(req->version);
    req->version = NULL;
    req->content = NULL;
}

int isConnectionKeepAlive(HttpReq *req)
{
    HttpHeader *header = findHeader(req->headers, "Connection");
    if (header == NULL)
    {
        if (getVersionNumber(req->version, 8) >= 11)
        {
            return 1;
        }
        return 0;
    }
    return strcasecmp(header->value, "keep-alive") == 0;
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
        .properties = allocate(sizeof(JProperty) * req->headers.count),
        .count = req->headers.count
    };
    HttpHeader *headers = req->headers.arr;
    for (int i = 0; i < req->headers.count; i++) {
        headersObj.properties[i] = _JProperty(headers[i].key, headers[i].value);
    }

    JToken contentToken;
    if (req->content == NULL) {
        contentToken = _JNull();
    } else {
        contentToken = _JToken((char *) req->content);
    }

    JObject reqObject = _JObject(
        _JProperty(methodKey, methodToStr(req->method)),
        _JProperty(pathKey, path),
        _JProperty(versionKey, req->version),
        _JProperty(headersKey, headersObj),
        _JProperty(contentKey, contentToken)
    );

    return reqObject;
}

void freeHttpReqJObject(JObject *req) {
    deallocate(req->properties[1].value.literal.string.value); //free path
    deallocate(req->properties[3].value.literal.object.properties);
    deallocate(req->properties);
}