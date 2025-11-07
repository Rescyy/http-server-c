//
// Created by Rescyy on 11/1/2025.
//

#include "http_parser.h"
#include <alloc.h>
#include "utils.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ERROR_MESSAGE_LENGTH 100u
#define HTTP_METHOD_MAX 7u
#define HTTP_PATH_MAX 1024u
#define HTTP_VERSION_MAX 8u
#define VERSION_LENGTH 8u
#define CRLF_LENGTH 2u
#define HEADER_SEPARATOR_LENGTH 2u

typedef enum  {
    INCOMPLETE = -1,
    NOT_FOUND = -2,
    INVALID = -3,
} StringFindResult;

static ssize_t findChar(const char *str, size_t size, char c, size_t max);
static size_t parseMethod(HttpReqParser *parser, const char *str, size_t size);
static size_t parsePath(HttpReqParser *parser, const char *str, size_t size);
static size_t parseVersion(HttpReqParser *parser, const char *str, size_t size);
static size_t parseHeaderKey(HttpReqParser *httpReqParser, const char *str, size_t size);
static size_t parseHeaderValue(HttpReqParser *httpReqParser, const char *str, size_t size);
static size_t parseContent(HttpReqParser *httpReqParser, const char *str, size_t size);
// static size_t

HttpReqParser *newHttpReqParser() {
    HttpReqParser *parser = allocate(sizeof(HttpReqParser));
    parser->state = INIT;
    parser->method = METHOD_UNKNOWN;
    parser->path = (HttpPath) {.raw = NULL, .elements = NULL, .elCount = 0};
    parser->version = NULL;
    parser->headers = (HttpHeaders) {.arr = NULL, .count = 0};
    parser->headersCapacity = 0;
    parser->content = NULL;
    parser->contentRead = 0;
    parser->contentLength = 0;
    parser->errorMessage = NULL;
    return parser;
}

size_t httpReqParserPush(HttpReqParser *parser, const void *buffer, const size_t size) {
    size_t consumed = 0;
    const char *str = buffer;

    while (1) {
        size_t stepConsumed = 0;

        switch (parser->state) {
            case INIT:
                parser->state = METHOD;
            case METHOD:
                stepConsumed = parseMethod(parser, str, size);
                break;
            case PATH:
                stepConsumed = parsePath(parser, str + consumed, size - consumed);
                break;
            case VERSION:
                break;
            case HEADER_KEY:
                break;
            case HEADER_VALUE:
                break;
            case CONTENT:
                break;
            default:
                return 0;
        }

        if (stepConsumed == 0) {
            return consumed;
        }

        if (parser->state == DONE) {
            return consumed + stepConsumed;
        }

        if (parser->state > INVALID_PROTOCOL) {
            return 0;
        }

        consumed += stepConsumed;
        parser->state++;
    }
}

static ssize_t findChar(const char *str, const size_t size, const char c, size_t max) {
    ssize_t errorReturn;
    if (max > size) {
        max = size;
        errorReturn = INCOMPLETE;
    } else {
        errorReturn = NOT_FOUND;
    }

    for (ssize_t i = 0; i < (ssize_t) max; i++) {
        if (str[i] == c) {
            return i;
        }
    }

    return errorReturn;
}

static size_t parseMethod(HttpReqParser *parser, const char *str, size_t size) {
    const int i = (int) findChar(str, size, ' ', HTTP_PATH_MAX + 1);
    if (i == INCOMPLETE) {
        return 0;
    }
    if (i == NOT_FOUND) {
        parser->state = METHOD_TOO_LONG;
        parser->errorMessage = allocate(ERROR_MESSAGE_LENGTH);
        snprintf(parser->errorMessage,
            ERROR_MESSAGE_LENGTH,
            "The method starting with '%.*s' is too long",
            (int) MIN(size, METHOD_TOO_LONG),
            str);
        return 0;
    }
    HttpMethod method = strnToMethod(str, i);
    if (method == METHOD_UNKNOWN) {
        parser->state = UNKNOWN_METHOD;
        parser->errorMessage = allocate(ERROR_MESSAGE_LENGTH);
        snprintf(parser->errorMessage,
            ERROR_MESSAGE_LENGTH,
            "Unknown method '%.*s'",
            i, str);
        return 0;
    }
    parser->method = method;
    return i + 1;
}

static size_t parsePath(HttpReqParser *parser, const char *str, size_t size) {
    HttpPath *path = &parser->path;
    size_t cursor = 0;
    char *element;
    int slashes = 0;
    if (size == 0 || str[cursor++] != '/') {
        return -1;
    }
    if (size == 1) {
        path->elements = NULL;
        return 0;
    }
    const int end = (int) findChar(str, size, ' ', HTTP_PATH_MAX + 1);
    if (end == INCOMPLETE) {
        return 0;
    }
    if (end == NOT_FOUND) {
        parser->state = PATH_TOO_LONG;
        parser->errorMessage = allocate(ERROR_MESSAGE_LENGTH);
        snprintf(parser->errorMessage,
            ERROR_MESSAGE_LENGTH,
            "The path starting with '.*s' is too long",
            (int) MIN(size, HTTP_PATH_MAX),
            str);
        return 0;
    }
    path->raw = allocate(size + 1);
    snprintf(path->raw, size, "%.*s", (int) size, str);
    for (size_t i = 0; i < size - 1; i++) {
        slashes += str[i] == '/';
    }

    path->elements = allocate(sizeof(path->elements[0]) * slashes);

    for (int i = 0; i < slashes - 1; i++) {
        for (size_t j = cursor; j < size; j++) {
            if (str[j] != '/') {
                continue;
            }
            element = allocate(j - cursor + 1);
            path->elements[path->elCount++] = element;
            snprintf(element, j - cursor + 1, "%.*s",
                (int)(j - cursor),
                str + cursor);
            cursor = j + 1;
            break;
        }
    }

    element = allocate(size - cursor + 1);
    path->elements[path->elCount++] = element;
    snprintf(element, size - cursor + 1, "%.*s",
        (int)(size - cursor),
        str + cursor);

    return end + 1;
}

static size_t parseVersion(HttpReqParser *parser, const char *str, size_t size) {
    if (size < VERSION_LENGTH) {
        return 0;
    }

    if (strncmp(str, "HTTP/", 5) != 0) {
        parser->state = VERSION_TOO_LONG;
        parser->errorMessage = allocate(ERROR_MESSAGE_LENGTH);
        snprintf(parser->errorMessage, "The version is not ");
    }

    return 0;
}
static size_t parseHeaderKey(HttpReqParser *httpReqParser, const char *str, size_t size) {
    return 0;
}
static size_t parseHeaderValue(HttpReqParser *httpReqParser, const char *str, size_t size) {
    return 0;
}
static size_t parseContent(HttpReqParser *httpReqParser, const char *str, size_t size) {
    return 0;
}
