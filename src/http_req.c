#include "http_req.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

int findContentLength(HttpHeaders headers);

/* Parse HTTP request, returns the amount of bytes needed additionally read from the TCP socket based on the Content-Length header */
int parseRequest(HttpReq *reqObj, void *reqBytes, int size)
{
    /* moving indeces */
    int offset, prevOffset = 0;

    offset = strnindex(reqBytes + prevOffset, size - prevOffset, " ");
    if (offset == -1)
    {
        return -1;
    }

    reqObj->method = strnToMethod(reqBytes + prevOffset, offset);
    if (reqObj->method == METHOD_UNKNOWN)
    {
        return -1;
    }

    prevOffset += offset + 1;

    offset = strnindex(reqBytes + prevOffset, size - prevOffset, " ");
    if (offset == -1)
    {
        return -1;
    }

    if (parsePath(&reqObj->path, reqBytes + prevOffset, offset) == -1)
    {
        return -1;
    }

    prevOffset += offset + 1;

    offset = strnindex(reqBytes + prevOffset, size - prevOffset, "\r\n");

    if (offset == -1 || !isVersionValid(reqBytes + prevOffset, offset))
    {
        freePath(&reqObj->path);
        return -1;
    }
    reqObj->version = malloc(offset + 1);
    snprintf(reqObj->version, offset + 1, "%s", (char *)reqBytes + prevOffset);

    prevOffset += offset + 2;

    offset = parseHeaders(&reqObj->headers, reqBytes + prevOffset, size - prevOffset);
    if (offset == -1)
    {
        freePath(&reqObj->path);
        free(reqObj->version);
        return -1;
    }
    // printf("prevOffset %d offset %d\n", prevOffset, offset);

    prevOffset += offset;

    reqObj->contentLength = findContentLength(reqObj->headers);
    if (reqObj->contentLength == -1)
    {
        reqObj->contentLength = size - prevOffset;
    }

    if (reqObj->contentLength == 0)
    {
        reqObj->content = NULL;
        reqObj->contentWritten = 0;
        return 0;
    }

    reqObj->content = malloc(reqObj->contentLength);
    reqObj->contentWritten = size - prevOffset;
    memcpy(reqObj->content, reqBytes + prevOffset, size - prevOffset);

    if (size - prevOffset < reqObj->contentLength)
    {
        return reqObj->contentLength - reqObj->contentWritten;
    }

    return 0;
}

void addAdditionalRequestContent(HttpReq *reqObj, void *additionalContent, int additionalBytes)
{
    assert(reqObj->contentWritten + additionalBytes <= reqObj->contentLength);

    reqObj->content = realloc(reqObj->content, reqObj->contentWritten);

    memcpy(reqObj->content + reqObj->contentWritten, additionalContent, additionalBytes);

    reqObj->contentWritten += additionalBytes;
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

    return -1;
}

const char *methodToStr(HttpMethod method)
{
    static const char get[] = "GET";
    static const char post[] = "POST";
    static const char unknown[] = "UNK";

    switch (method)
    {
    case GET:
        return get;
    case POST:
        return post;
    default:
        return unknown;
    }
}

HttpMethod strnToMethod(const char *str, int n)
{
#define IS_METHOD(method)                          \
    if (strncmp(methodToStr(method), str, n) == 0) \
    return (method)
    IS_METHOD(GET);
    IS_METHOD(POST);
    return METHOD_UNKNOWN;
#undef IS_METHOD
}

// #define DEBUG_STR_TO_PATH
#ifdef DEBUG_STR_TO_PATH
#endif
int parsePath(HttpPath *path, const char *str, int n)
{
    int offset, prevOffset = 0, pathCap = 1;
    path->elCount = 0;
    path->elements = NULL;
    char *element;
    if (str[prevOffset++] != '/')
        return -1;

    path->elements = malloc(sizeof(char *));

    while ((offset = strnindex(str + prevOffset, n - prevOffset, "/")) != -1)
    {
        element = malloc(offset + 1);

        if (path->elCount >= pathCap)
        {
            pathCap *= 2;
            path->elements = realloc(path->elements, sizeof(char *) * pathCap);
        }

        path->elements[path->elCount++] = element;
        snprintf(element, offset + 1, "%s", str + prevOffset);

        prevOffset += offset + 1;
    }

    int elCap = n - prevOffset + 1;

#ifdef DEBUG_STR_TO_PATH
    printf("n prevOffset %d %d\n", n, prevOffset);
#endif

    if (elCap > 1)
    {
        element = malloc(elCap);
        if (path->elCount == pathCap)
        {
            pathCap++;
            path->elements = realloc(path->elements, sizeof(char *) * pathCap);
        }
        path->elements[path->elCount++] = element;
        snprintf(element, elCap, "%s", str + prevOffset);
    }

    if (path->elCount < pathCap)
    {
        path->elements = realloc(path->elements, sizeof(char *) * pathCap);
    }

    return 0;
}

void freePath(HttpPath *path)
{
    for (int i = 0; i < path->elCount; i++)
    {
        free(path->elements[i]);
    }
    free(path->elements);
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
            int result = strtol(reqPath.elements[i], NULL, 10);
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

int pathToStr(char *str, int size, HttpPath path)
{
    if (size >= 2 && path.elCount == 0)
    {
        return snprintf(str, size, "/");
    }

    int bytes = 0;
    for (int i = 0; i < path.elCount; i++)
    {
        bytes += snprintf(str + bytes, size - bytes, "/%s", path.elements[i]);
    }
    return bytes;
}

int reqEq(HttpReq obj1, HttpReq obj2)
{ /*miau*/
    assert(reqIsComplete(obj1) == 1);
    assert(reqIsComplete(obj2) == 1);

    // printf("%s %s\n", methodToStr(obj1.method), methodToStr(obj2.method));
    if (obj1.method != obj2.method)
    {
        return 0;
    }

    // printf("pathEq %d\n", pathEq(obj1.path, obj2.path));
    if (!pathEq(obj1.path, obj2.path))
    {
        return 0;
    }

    // printf("version %s %s\n", obj1.version, obj2.version);
    if (strcmp(obj1.version, obj2.version) != 0)
    {
        return 0;
    }

    // printf("headers %d\n", headersEq(obj1.headers, obj2.headers));
    if (!headersEq(obj1.headers, obj2.headers))
    {
        return 0;
    }

    // printf("contentLength %d %d\n", obj1.contentLength, obj2.contentLength);
    if (obj1.contentLength != obj2.contentLength)
    {
        return 0;
    }

    // printf("content %s %s\n", (char *)obj1.content, (char *)obj2.content);
    if (obj1.contentLength == 0 && obj2.contentLength == 0)
    {
        return 1;
    }

    if (obj1.content == NULL || obj2.content == NULL || obj1.contentWritten != obj2.contentWritten || memcmp(obj1.content, obj2.content, obj1.contentLength) != 0)
    {
        return 0;
    }

    return 1;
}

int reqIsComplete(HttpReq req)
{
    return req.contentWritten == req.contentLength;
}

void freeReq(HttpReq *req)
{
    freePath(&req->path);
    freeHeaders(&req->headers);
    free(req->version);
    free(req->content);
    req->version = NULL;
    req->content = NULL;
}