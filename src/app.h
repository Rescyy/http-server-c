//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef APP_H
#define APP_H

#include "http.h"
#include "http_req.h"
#include "http_resp.h"

typedef HttpResp (*HttpReqHandler) (HttpReq);

/*
    asserts whether the app started or not succesfully
*/
void startApp(char* port);
void addEndpoint(char *path, HttpReqHandler handler);
void setNotFoundCallback(HttpReqHandler handler);
void setLogFile(const char *path);


#endif //APP_H
