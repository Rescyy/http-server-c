#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include "http_req.h"
#include "http_resp.h"

typedef HttpResp (*HttpReqHandler)(HttpReq);

typedef struct HttpEndpoint {
    HttpPath path;
    HttpReqHandler handler;
} HttpEndpoint;

typedef struct HttpRouter {
    HttpEndpoint *endpoints;
    HttpReqHandler notFoundCallback;
    int length;
    int capacity;
} HttpRouter;

HttpResp routeReq(HttpRouter router, HttpReq req);
HttpEndpoint newEndpoint(char *str, HttpReqHandler handler);
HttpRouter newRouter(HttpEndpoint *endpoints, int length);
HttpRouter emptyRouter();
void routerAddEndpoint(HttpRouter *router, HttpEndpoint endpoint);
void freeEndpoint(HttpEndpoint *endpoint);
void freeRouter(HttpRouter *router);

#endif