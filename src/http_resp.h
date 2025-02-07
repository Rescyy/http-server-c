#ifndef HTTP_RESP_H
#define HTTP_RESP_H

#include "http.h"
#include <stdio.h>

#define HEADER_KEY_SIZE_LIMIT 64
#define HEADER_VALUE_SIZE_LIMIT 1024

typedef enum HttpStatus {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
    STATUS_UNKNOWN = -1,
} HttpStatus;

typedef struct HttpResp {
    char *version;
    HttpStatus status;
    HttpHeaders headers;
    void *content;
    int contentLength;
} HttpResp;

typedef enum HttpContentType
{
    TEXT_PLAIN,
    TEXT_HTML,
    CONTENT_UNKNOWN = -1,
} HttpContentType;

typedef struct HttpRespBuilder {
    HttpResp resp;
    int headersCapacity;
} HttpRespBuilder;

const char *statusToStr(HttpStatus status);
HttpStatus strnToStatus(const char *str, int n);
int respToStr(HttpResp resp, char *str, int size);
int respFirstLineStr(HttpResp *resp, char *str, int size);
int respUntilEmptyLineStr(HttpResp *resp, char *str, int size);
int respEq(HttpResp obj1, HttpResp obj2);
void freeResp(HttpResp *resp);

HttpRespBuilder newRespBuilder();
void respBuilderSetStatus(HttpRespBuilder *builder, HttpStatus status);
void respBuilderSetVersion(HttpRespBuilder *builder, const char *version);
void respBuilderAddHeader(HttpRespBuilder *builder, char *key, char *value);
void respBuilderSetContent(HttpRespBuilder *builder, void *content, int contentLength);
void respBuilderSetFileContent(HttpRespBuilder *builder, const char *path);
HttpResp respBuild(HttpRespBuilder *builder);

#endif