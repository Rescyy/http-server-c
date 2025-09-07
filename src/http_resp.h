//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef HTTP_RESP_H
#define HTTP_RESP_H

#include "http.h"

#define HEADER_KEY_SIZE_LIMIT 64
#define HEADER_VALUE_SIZE_LIMIT 1024

typedef enum HttpStatus {
    CONTINUE = 100,
    SWITCHING_PROTOCOLS = 101,
    PROCESSING = 102,
    EARLY_HINTS = 103,
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NON_AUTHORITATIVE_INFORMATION = 203,
    NO_CONTENT = 204,
    RESET_CONTENT = 205,
    PARTIAL_CONTENT = 206,
    MULTI_STATUS = 207,
    ALREADY_REPORTED = 208,
    IM_USED = 226,
    MULTIPLE_CHOICES = 300,
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    SEE_OTHER = 303,
    NOT_MODIFIED = 304,
    USE_PROXY = 305,
    TEMPORARY_REDIRECT = 307,
    PERMANENT_REDIRECT = 308,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    PAYMENT_REQUIRED = 402,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    NOT_ACCEPTABLE = 406,
    PROXY_AUTHENTICATION_REQUIRED = 407,
    REQUEST_TIMEOUT = 408,
    CONFLICT = 409,
    GONE = 410,
    LENGTH_REQUIRED = 411,
    PRECONDITION_FAILED = 412,
    PAYLOAD_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    UNSUPPORTED_MEDIA_TYPE = 415,
    RANGE_NOT_SATISFIABLE = 416,
    EXPECTATION_FAILED = 417,
    IM_A_TEAPOT = 418,
    MISDIRECTED_REQUEST = 421,
    UNPROCESSABLE_ENTITY = 422,
    LOCKED = 423,
    FAILED_DEPENDENCY = 424,
    TOO_EARLY = 425,
    UPGRADE_REQUIRED = 426,
    PRECONDITION_REQUIRED = 428,
    TOO_MANY_REQUESTS = 429,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    UNAVAILABLE_FOR_LEGAL_REASONS = 451,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    VARIANT_ALSO_NEGOTIATES = 506,
    INSUFFICIENT_STORAGE = 507,
    LOOP_DETECTED = 508,
    NOT_EXTENDED = 510,
    NETWORK_AUTHENTICATION_REQUIRED = 511,
    STATUS_UNKNOWN = -1,
} HttpStatus;

typedef struct HttpResp {
    char *version;
    HttpStatus status;
    HttpHeaders headers;
    void *content;
    size_t contentLength;
    int isContentFile;
} HttpResp;

typedef enum HttpMimeType
{
    TEXT_PLAIN,
    TEXT_HTML,
    CONTENT_UNKNOWN = -1,
} HttpContentType;

#define USE_DEFAULT_SERVER_HEADER_FLAG 1 << 0
#define USE_NO_CONTENT_RESPONSE_FLAG 1 << 1

typedef struct HttpRespBuilder {
    HttpResp resp;
    int headersCapacity;
    unsigned int flags;
} HttpRespBuilder;

const char *statusToStr(HttpStatus status);
HttpStatus strnToStatus(const char *str, int n);
int respToStr(HttpResp resp, char *str, int size);
int buildRespStringFirstLineStr(HttpResp *resp, char *str, int size);
int buildRespStringUntilContent(HttpResp *resp, char *str, int size);
int respEq(HttpResp obj1, HttpResp obj2);
void freeResp(HttpResp *resp);

HttpResp newResp(HttpStatus status);
HttpRespBuilder newRespBuilder();
void respBuilderSetStatus(HttpRespBuilder *builder, HttpStatus status);
void respBuilderSetVersion(HttpRespBuilder *builder, const char *version);
void respBuilderAddHeader(HttpRespBuilder *builder, char *key, char *value);
void respBuilderSetContent(HttpRespBuilder *builder, void *content, size_t contentLength);
void respBuilderSetFileContent(HttpRespBuilder *builder, const char *path);
#define SET_FLAGS 0
#define UNSET_FLAGS 1
#define REPLACE_FLAGS 2
void respBuilderSetFlags(HttpRespBuilder *builder, unsigned int flags, int behaviour);
void respBuilderSetDefaultFlags(unsigned int flags);
HttpResp respBuild(HttpRespBuilder *builder);

#endif //HTTP_RESP_H
