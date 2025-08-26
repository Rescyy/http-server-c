//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#ifndef LOGGING_H
#define LOGGING_H

#include "connection.h"
#include "http_req.h"
#include "http_resp.h"

#define PRINT_LOG 1 << 0
#define FILE_LOG 1 << 1
#define JSON_LOG 1 << 2

void setLogFile(const char *path);
void setLogFlags(int flags);
void logResponse(HttpResp *resp, HttpReq *req);
void logError(HttpReq *req, const char *error);
void printError(HttpReq *req, TcpSocket *client);

#endif //LOGGING_H
