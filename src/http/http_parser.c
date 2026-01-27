//
// Created by Rescyy on 11/1/2025.
//

#include "http_parser.h"
#include <alloc.h>
#include "utils.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ERROR_MESSAGE_LENGTH 100u
#define METHOD_MAX 7u
#define HTTP_PATH_MAX 1024u
#define VERSION_LENGTH 8u
#define HEADER_KEY_MAX 4096u
#define HEADER_VALUE_MAX 4096u
#define CRLF "\r\n"
#define CRLF_LENGTH strlen(CRLF)
#define HEADER_SEPARATOR ": "
#define HEADER_SEPARATOR_LENGTH strlen(HEADER_SEPARATOR)

DEFINE_ARRAY_C(HttpHeader)

typedef enum  {
    INCOMPLETE = -1,
    NOT_FOUND = -2,
    INVALID = -3,
} StringFindResult;

static size_t parseMethod(HttpReqParser *parser, const char *str, size_t size);
static size_t parsePath(HttpReqParser *parser, const char *str, size_t size);
static size_t parseVersion(HttpReqParser *parser, const char *str, size_t size);
static size_t parseHeaderKey(HttpReqParser *parser, const char *str, size_t size);
static size_t parseHeaderValue(HttpReqParser *parser, const char *str, size_t size);
static size_t parseContent(HttpReqParser *parser, const char *str, size_t size);
static ssize_t findChar(const char *str, size_t size, char c, size_t max);
static ssize_t findString(const char *str, size_t size, const char *subString, size_t max);
static int findContentLength(HttpReqParser *parser);

HttpReqParser *newHttpReqParser() {
    HttpReqParser *parser = allocate(sizeof(HttpReqParser));
    parser->state = INIT;
    parser->method = METHOD_UNKNOWN;
    parser->path = (HttpPath) {.raw = NULL, .elements = NULL, .elCount = 0};
    parser->version = NULL;
    parser->headers = ARRAY_WITH_CAPACITY(HttpHeader, 0);
    parser->content = NULL;
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
                stepConsumed = parseVersion(parser, str + consumed, size - consumed);
                break;
            case HEADER_KEY:
                stepConsumed = parseHeaderKey(parser, str + consumed, size);
                break;
            case HEADER_VALUE:
                stepConsumed = parseHeaderValue(parser, str + consumed, size - consumed);
                break;
            case CONTENT:
                stepConsumed = parseContent(parser, str + consumed, size - consumed);
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

    for (size_t i = 0; i < max; i++) {
        if (str[i] == c) {
            return (ssize_t) i;
        }
        if (str[i] < ' ') {
            return INVALID;
        }
    }

    return errorReturn;
}

static ssize_t findString(const char *str, const size_t size, const char *subString, size_t max) {
    ssize_t errorReturn;
    if (max > size) {
        max = size;
        errorReturn = INCOMPLETE;
    } else {
        errorReturn = NOT_FOUND;
    }

    size_t subStringLength = strlen(subString);
    size_t to = max - subStringLength + 1;

    for (size_t i = 0; i < to; i++) {
        if (strncmp(str + i, subString, subStringLength) == 0) {
            return (ssize_t) i;
        }
        if (str[i] < ' ') {
            return INVALID;
        }
    }

    return errorReturn;
}

static int findContentLength(HttpReqParser *parser) {
    static const char contentLength[] = "content-length";

    if (parser->contentLength > 0) {
        return 1;
    }

    for (unsigned int i = 0; i < parser->headers.length; i++) {
        const HttpHeader *header = &parser->headers.data[i];

        if (header->key.length != strlen(contentLength)) {
            continue;
        }

        if (strncasecmp(header->key.ptr, contentLength, strlen(contentLength)) == 0) {
            const unsigned long long value = strtoull(header->value.ptr, NULL, 10);
            if (errno == ERANGE || value == ULLONG_MAX) {
                return 0;
            }
            parser->contentLength = value;

            return 1;
        }
    }
    return 1;
}

static size_t parseMethod(HttpReqParser *parser, const char *str, size_t size) {
    const int i = (int) findChar(str, size, ' ', METHOD_MAX + 1);
    switch (i) {
        case INCOMPLETE:
            return 0;
        case NOT_FOUND:
            parser->state = METHOD_TOO_LONG;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
            snprintf(parser->errorMessage,
                ERROR_MESSAGE_LENGTH,
                "The method starting with '%.*s' is too long",
                (int) METHOD_TOO_LONG,
                str);
            return 0;
        case INVALID:
            parser->state = INVALID_METHOD;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
            snprintf(parser->errorMessage,
                ERROR_MESSAGE_LENGTH,
                "The method contains invalid characters");
            return 0;
        default:
            break;
    }

    HttpMethod method = strnToMethod(str, i);
    if (method == METHOD_UNKNOWN) {
        parser->state = INVALID_METHOD;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage,
            ERROR_MESSAGE_LENGTH,
            "Unknown method '%.*s'",
            i, str);
        return 0;
    }
    parser->method = method;
    parser->state = PATH;
    return i + 1;
}

static size_t parsePath(HttpReqParser *parser, const char *str, size_t size) {
    HttpPath *path = &parser->path;
    size_t cursor = 0;
    char *element;
    int slashes = 0;
    if (size == 0) {
        return 0;
    }
    if (str[cursor++] != '/') {
        parser->state = INVALID_PATH;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage, ERROR_MESSAGE_LENGTH, "Expected path to start with slash '/'");
        return 0;
    }
    if (size == 1) {
        path->elements = NULL;
        return 0;
    }
    const ssize_t end = findChar(str, size, ' ', HTTP_PATH_MAX + 1);
    switch (end) {
        case INCOMPLETE:
            return 0;
        case NOT_FOUND:
            parser->state = PATH_TOO_LONG;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH + HTTP_PATH_MAX, 1);
            snprintf(parser->errorMessage,
                ERROR_MESSAGE_LENGTH + HTTP_PATH_MAX,
                "The path starting with '%.*s' is too long",
                (int) HTTP_PATH_MAX,
                str);
            return 0;
        case INVALID:
            parser->state = INVALID_PATH;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
            snprintf(parser->errorMessage,
                ERROR_MESSAGE_LENGTH,
                "The path contains invalid characters");
            return 0;
        default:
            break;
    }

    path->raw = gcArenaAllocate(end + 1, 1);

    snprintf(path->raw, end + 1, "%.*s", (int) end, str);
    for (size_t i = 0; i < size - 1; i++) {
        slashes += str[i] == '/';
    }

    path->elements = gcArenaAllocate(sizeof(HttpPath *) * slashes, sizeof(HttpPath *));
    for (int i = 0; i < slashes - 1; i++) {
        for (size_t j = cursor; j < size; j++) {
            if (str[j] != '/') {
                continue;
            }
            element = gcArenaAllocate(j - cursor + 1, sizeof(char));
            path->elements[path->elCount++] = element;
            snprintf(element, j - cursor + 1, "%.*s",
                (int)(j - cursor),
                str + cursor);
            cursor = j + 1;
            break;
        }
    }

    element = gcArenaAllocate(size - cursor + 1, sizeof(char));
    path->elements[path->elCount++] = element;
    snprintf(element, size - cursor + 1, "%.*s",
        (int)(size - cursor),
        str + cursor);

    parser->state = VERSION;
    return end + 1;
}

static size_t parseVersion(HttpReqParser *parser, const char *str, size_t size) {
    if (size < VERSION_LENGTH + CRLF_LENGTH) {
        return 0;
    }

    if (strncmp(str + VERSION_LENGTH, CRLF, 2) != 0) {
        parser->state = VERSION_TOO_LONG;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage,
                 ERROR_MESSAGE_LENGTH,
                 "The version starting with '%.*s' is too long",
                 (int) VERSION_LENGTH,
                 str);
        return 0;
    }

    if (strncmp(str, "HTTP/", 5) != 0) {
        parser->state = INVALID_VERSION;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage,
                 ERROR_MESSAGE_LENGTH,
                 "The version was expected to start with 'HTTP/'");
        return 0;
    }

    if (strncmp(str + 5, "0.9", 3) == 0) {
        parser->version = "HTTP/0.9";
    } else if (strncmp(str + 5, "1.0", 3) == 0) {
        parser->version = "HTTP/1.0";
    } else if (strncmp(str + 5, "1.1", 3) == 0) {
        parser->version = "HTTP/1.1";
    } else if (strncmp(str + 5, "2.0", 3) == 0) {
        parser->version = "HTTP/2.0";
    } else if (strncmp(str + 5, "3.0", 3) == 0) {
        parser->version = "HTTP/3.0";
    } else {
        parser->state = INVALID_VERSION;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage,
            ERROR_MESSAGE_LENGTH,
            "Unknown http version '%.*s'",
            (int) VERSION_LENGTH, str);
        return 0;
    }

    parser->state = HEADER_KEY;
    return VERSION_LENGTH + CRLF_LENGTH;
}

static size_t parseHeaderKey(HttpReqParser *parser, const char *str, size_t size) {
    if (strncmp(str, CRLF, CRLF_LENGTH) == 0) {
        parser->state = CONTENT;
        return CRLF_LENGTH;
    }

    const int i = (int) findString(str, size, HEADER_SEPARATOR, HEADER_KEY_MAX + HEADER_SEPARATOR_LENGTH);

    switch (i) {
        case INCOMPLETE:
            return 0;
        case NOT_FOUND:
            parser->state = HEADER_KEY_TOO_LONG;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH + HEADER_KEY_MAX, 1);
            snprintf(parser->errorMessage,
                     ERROR_MESSAGE_LENGTH + HEADER_KEY_MAX,
                     "The header key starting with '%.*s' is too long. Max allowed: %d",
                     (int) HEADER_KEY_MAX,
                     str,
                     HEADER_KEY_MAX);
            return 0;
        case INVALID:
            parser->state = INVALID_HEADER_KEY;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
            snprintf(parser->errorMessage,
                     ERROR_MESSAGE_LENGTH,
                     "The header key contains invalid characters");
            return 0;
        default:
            break;
    }

    string key = {.ptr = (char*) str, .length = i};
    HttpHeader header = {.key = key, .value = EMPTY_STRING};

    ARRAY_PUSH(HttpHeader, &parser->headers, header);

    parser->state = HEADER_VALUE;
    return i + HEADER_SEPARATOR_LENGTH;
}

static size_t parseHeaderValue(HttpReqParser *parser, const char *str, size_t size) {
    const int i = (int) findString(str, size, CRLF, HEADER_VALUE_MAX + CRLF_LENGTH);

    switch (i) {
        case INCOMPLETE:
            return 0;
        case NOT_FOUND:
            parser->state = HEADER_KEY_TOO_LONG;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH + HEADER_VALUE_MAX, 1);
            snprintf(parser->errorMessage,
                     ERROR_MESSAGE_LENGTH + HEADER_VALUE_MAX,
                     "The header value starting with '%.*s' is too long. Max allowed: %d",
                     (int) HEADER_VALUE_MAX,
                     str,
                     HEADER_VALUE_MAX
            );
            return 0;
        case INVALID:
            parser->state = INVALID_HEADER_VALUE;
            parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
            snprintf(parser->errorMessage,
                ERROR_MESSAGE_LENGTH,
                "The header value contains invalid characters");
            return 0;
        default:
            break;
    }

    string value = { .ptr = (char*) str, .length = i};
    ARRAY_T(HttpHeader) *headers = &parser->headers;
    HttpHeader *header = &headers->data[headers->length - 1];
    header->value = value;
    parser->state = HEADER_KEY;

    return i + HEADER_SEPARATOR_LENGTH;
}

static size_t parseContent(HttpReqParser *parser, const char *str, size_t size) {
    if (!findContentLength(parser)) {
        parser->state = INVALID_CONTENT_LENGTH;
        parser->errorMessage = gcArenaAllocate(ERROR_MESSAGE_LENGTH, 1);
        snprintf(parser->errorMessage, ERROR_MESSAGE_LENGTH, "Invalid content length header");
    }

    if (parser->contentLength == 0) {
        parser->state = DONE;
        return 0;
    }

    if (parser->contentLength <= size) {
        parser->content = (void*) str;
        parser->state = DONE;
        return parser->contentLength;
    }

    return 0;
}
