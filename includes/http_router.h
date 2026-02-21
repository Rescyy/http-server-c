//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_ROUTER_H
#define HTTP_ROUTER_H

#include "http_req.h"
#include "http_resp.h"

typedef HttpResp (*HttpReqHandler)(HttpReq);

typedef struct HttpEndpoint {
    HttpPath path;
    HttpReqHandler handler;
    const char* raw;
} HttpEndpoint;

typedef struct HttpRouter {
    HttpEndpoint *endpoints;
    HttpReqHandler notFoundCallback;
    int length;
    int capacity;
} HttpRouter;

HttpResp routeReq(HttpRouter *router, HttpReq *req);
HttpEndpoint newEndpoint(const char *str, HttpReqHandler handler);
HttpRouter newRouter(HttpEndpoint *endpoints, int length);
HttpRouter emptyRouter();
void routerAddEndpoint(HttpRouter *router, HttpEndpoint endpoint);
void freeEndpoint(HttpEndpoint *endpoint);
void freeRouter(HttpRouter *router);

#endif //HTTP_ROUTER_H
