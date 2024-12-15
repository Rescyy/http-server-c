#ifndef APP_H
#define APP_H

/*
    asserts whether the app started or not succesfully
*/
#include "http.h"
#include "http_req.h"
#include "http_resp.h"

typedef HttpResp (*HttpReqHandler) (HttpReq);

void startApp(char* port);
void addEndpoint(char *path, HttpReqHandler handler);
void setNotFoundCallback(HttpReqHandler handler);

#endif