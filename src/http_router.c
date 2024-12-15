#include "http_router.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static HttpResp defaultNotFoundCallback(HttpReq req)
{
    HttpRespBuilder builder = newRespBuilder();
    char notFoundContent[] = "<!DOCTYPE html><body><head>NOT FOUND 404</head></body>";
    HttpResp resp = builder.setStatus(&builder, NOT_FOUND)
    ->setContent(&builder, notFoundContent, sizeof(notFoundContent) - 1)
    ->addHeader(&builder, "Content-Type", "text/html")
    ->build(&builder);
    return resp;
}

HttpResp routeReq(HttpRouter router, HttpReq req)
{
    for (int i = 0; i < router.length; i++)
    {
        HttpEndpoint endpoint = router.endpoints[i];
        if (pathMatches(endpoint.path, req.path))
        {
            return endpoint.handler(req);
        }
    }
    return router.notFoundCallback(req);
}

HttpEndpoint newEndpoint(char *str, HttpReqHandler handler)
{
    HttpPath path;
    int status = parsePath(&path, str, strlen(str));
    assert(status != -1 && "Programmer error: Invalid path passed");
    return (HttpEndpoint){path, handler};
}

/*
    The endpoints must have a lifetime longer or equal than the router.
    No copies are made.
*/
HttpRouter newRouter(HttpEndpoint *endpoints, int length)
{
    assert(endpoints != NULL || (endpoints == NULL && length == 0));
    return (HttpRouter){endpoints, defaultNotFoundCallback, length, -1};
}

HttpRouter emptyRouter()
{
    return (HttpRouter){NULL, defaultNotFoundCallback, 0, 0};
}

void routerAddEndpoint(HttpRouter *router, HttpEndpoint endpoint)
{
    assert(router != NULL);
    if (router->endpoints == NULL)
    {
        router->endpoints = malloc(sizeof(HttpEndpoint));
        router->capacity = 1;
    }
    if (router->length >= router->capacity)
    {
        router->capacity *= 2;
        router->endpoints = realloc(router->endpoints, router->capacity * sizeof(HttpEndpoint));
    }
    router->endpoints[router->length++] = endpoint;
}

void freeEndpoint(HttpEndpoint *endpoint)
{
    freePath(&endpoint->path);
    endpoint->handler = NULL;
}

void freeRouter(HttpRouter *router)
{
    for (int i = 0; i < router->length; i++)
    {
        freeEndpoint(&router->endpoints[i]);
    }
    router->endpoints = NULL;
    router->notFoundCallback = NULL;
    router->length = 0;
}