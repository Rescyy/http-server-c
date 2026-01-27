//
// Created by Rescyy on 11/1/2025.
//

#ifndef HTTPSERVERC_HTTP_PARSER_H
#define HTTPSERVERC_HTTP_PARSER_H

#include "http_req.h"
#include "array.h"

typedef enum {
    INIT = 0,
    METHOD,
    PATH,
    VERSION,
    HEADER_KEY,
    HEADER_VALUE,
    CONTENT,
    DONE,

    INVALID_PROTOCOL = 100,
    INVALID_METHOD,
    INVALID_PATH,
    INVALID_VERSION,
    INVALID_HEADER_KEY,
    INVALID_HEADER_VALUE,
    INVALID_CONTENT_LENGTH,

    ENTITY_TOO_LONG = 200,
    METHOD_TOO_LONG,
    PATH_TOO_LONG,
    VERSION_TOO_LONG,
    HEADER_KEY_TOO_LONG,
    HEADER_VALUE_TOO_LONG,

} HttpReqParserState;

DEFINE_ARRAY_H(HttpHeader)

typedef struct {
    HttpReqParserState state;
    HttpMethod method;
    HttpPath path;
    char *version;
    ARRAY_T(HttpHeader) headers;
    void *content;
    size_t contentLength;
    char *errorMessage;
} HttpReqParser;

HttpReqParser *newHttpReqParser();
/*
 * Returns the amount of consumed bytes.
 * Takes in a parser with a valid state.
 */
size_t httpReqParserPush(HttpReqParser *parser, const void *buffer, size_t size);

#endif //HTTPSERVERC_HTTP_PARSER_H