//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <alloc.h>
#include <assert.h>
#include <http_resp.h>
#include <http_router.h>
#include <stdio.h>
#include <string.h>

static HttpResp defaultNotFoundCallback(HttpReq _)
{
    HttpRespBuilder builder = newRespBuilder();
    respBuilderSetStatus(&builder, NOT_FOUND);
    HttpResp resp = respBuild(&builder);
    return resp;
}

HttpResp htmlNotFoundCallback(HttpReq _) {
    HttpRespBuilder builder = newRespBuilder();
    static const char notFoundContent[] = "<!DOCTYPE html><body><head>NOT FOUND 404</head></body>";
    respBuilderSetStatus(&builder, NOT_FOUND);
    respBuilderSetContent(&builder, notFoundContent, sizeof(notFoundContent) - 1, 0);
    respBuilderAddHeader(&builder, "Content-Type", "text/html");
    HttpResp resp = respBuild(&builder);
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
        router->endpoints = gcAllocate(sizeof(HttpEndpoint));
        router->capacity = 1;
    }
    if (router->length >= router->capacity)
    {
        router->capacity *= 2;
        router->endpoints = gcReallocate(router->endpoints, router->capacity * sizeof(HttpEndpoint));
    }
    router->endpoints[router->length++] = endpoint;
}
