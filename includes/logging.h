//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#ifndef LOGGING_H
#define LOGGING_H

#define LOG_REQUESTS 1
#define LOG_BY_LEVEL 1
#define LOG_TXT_FILE 0
#define LOG_JSON_FILE 0
#define LOG_TRACES 0

#if LOG_TRACES
#define TRACE(format, ...) trace(__FILE__, __LINE__, format, __VA_ARGS__)
#else
#define TRACE(_,...)
#endif

#include "connection.h"
#include "http_req.h"
#include "http_resp.h"

#define PRINT_LOG 1 << 0
#define FILE_LOG 1 << 1
#define JSON_LOG 1 << 2

typedef enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    TRACE_LOGLEVEL,
} LogLevel;

void trace(const char *file, int line, const char *format, ...);
void debug(const char *format, ...);
void info(const char *format, ...);
void warning(const char *format, ...);
void error(const char *format, ...);
void fatal(const char *format, ...);
void setLogFile(const char *path);
void setJsonLogFile(const char *path);
void setSocketLogFile(const char *path);
void setLogFlags(int flags);
void logResponse(HttpResp *resp, HttpReq *req);
void logErrorResponse(HttpReq *req, const char *error);
void initLogging();

#endif //LOGGING_H
