//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <http_resp.h>
#include <file_handler.h>
#include <alloc.h>
#include <utils.h>

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

#include "../../includes/logging.h"

const char *statusToStr(HttpStatus status)
{
    static const char ok[] = "OK";
    static const char continueStr[] = "Continue";
    static const char switchingProtocols[] = "Switching Protocols";
    static const char processing[] = "Processing";
    static const char earlyHints[] = "Early Hints";
    static const char created[] = "Created";
    static const char accepted[] = "Accepted";
    static const char nonAuthoritativeInformation[] = "Non-Authoritative Information";
    static const char noContent[] = "No Content";
    static const char resetContent[] = "Reset Content";
    static const char partialContent[] = "Partial Content";
    static const char multiStatus[] = "Multi-Status";
    static const char alreadyReported[] = "Already Reported";
    static const char imUsed[] = "IM Used";
    static const char multipleChoices[] = "Multiple Choices";
    static const char movedPermanently[] = "Moved Permanently";
    static const char found[] = "Found";
    static const char seeOther[] = "See Other";
    static const char notModified[] = "Not Modified";
    static const char useProxy[] = "Use Proxy";
    static const char temporaryRedirect[] = "Temporary Redirect";
    static const char permanentRedirect[] = "Permanent Redirect";
    static const char badRequest[] = "Bad Request";
    static const char unauthorized[] = "Unauthorized";
    static const char paymentRequired[] = "Payment Required";
    static const char forbidden[] = "Forbidden";
    static const char notFound[] = "Not Found";
    static const char methodNotAllowed[] = "Method Not Allowed";
    static const char notAcceptable[] = "Not Acceptable";
    static const char proxyAuthenticationRequired[] = "Proxy Authentication Required";
    static const char requestTimeout[] = "Request Timeout";
    static const char conflict[] = "Conflict";
    static const char gone[] = "Gone";
    static const char lengthRequired[] = "Length Required";
    static const char preconditionFailed[] = "Precondition Failed";
    static const char payloadTooLarge[] = "Payload Too Large";
    static const char uriTooLong[] = "URI Too Long";
    static const char unsupportedMediaType[] = "Unsupported Media Type";
    static const char rangeNotSatisfiable[] = "Range Not Satisfiable";
    static const char expectationFailed[] = "Expectation Failed";
    static const char imATeapot[] = "I'm a teapot";
    static const char misdirectedRequest[] = "Misdirected Request";
    static const char unprocessableEntity[] = "Unprocessable Entity";
    static const char locked[] = "Locked";
    static const char failedDependency[] = "Failed Dependency";
    static const char tooEarly[] = "Too Early";
    static const char upgradeRequired[] = "Upgrade Required";
    static const char preconditionRequired[] = "Precondition Required";
    static const char tooManyRequests[] = "Too Many Requests";
    static const char requestHeaderFieldsTooLarge[] = "Request Header Fields Too Large";
    static const char unavailableForLegalReasons[] = "Unavailable For Legal Reasons";
    static const char internalServerError[] = "Internal Server Error";
    static const char notImplemented[] = "Not Implemented";
    static const char badGateway[] = "Bad Gateway";
    static const char serviceUnavailable[] = "Service Unavailable";
    static const char gatewayTimeout[] = "Gateway Timeout";
    static const char httpVersionNotSupported[] = "HTTP Version Not Supported";
    static const char variantAlsoNegotiates[] = "Variant Also Negotiates";
    static const char insufficientStorage[] = "Insufficient Storage";
    static const char loopDetected[] = "Loop Detected";
    static const char notExtended[] = "Not Extended";
    static const char networkAuthenticationRequired[] = "Network Authentication Required";

    switch (status)
    {
    case CONTINUE:
        return continueStr;
    case SWITCHING_PROTOCOLS:
        return switchingProtocols;
    case PROCESSING:
        return processing;
    case EARLY_HINTS:
        return earlyHints;
    case OK:
        return ok;
    case CREATED:
        return created;
    case ACCEPTED:
        return accepted;
    case NON_AUTHORITATIVE_INFORMATION:
        return nonAuthoritativeInformation;
    case NO_CONTENT:
        return noContent;
    case RESET_CONTENT:
        return resetContent;
    case PARTIAL_CONTENT:
        return partialContent;
    case MULTI_STATUS:
        return multiStatus;
    case ALREADY_REPORTED:
        return alreadyReported;
    case IM_USED:
        return imUsed;
    case MULTIPLE_CHOICES:
        return multipleChoices;
    case MOVED_PERMANENTLY:
        return movedPermanently;
    case FOUND:
        return found;
    case SEE_OTHER:
        return seeOther;
    case NOT_MODIFIED:
        return notModified;
    case USE_PROXY:
        return useProxy;
    case TEMPORARY_REDIRECT:
        return temporaryRedirect;
    case PERMANENT_REDIRECT:
        return permanentRedirect;
    case BAD_REQUEST:
        return badRequest;
    case UNAUTHORIZED:
        return unauthorized;
    case PAYMENT_REQUIRED:
        return paymentRequired;
    case FORBIDDEN:
        return forbidden;
    case NOT_FOUND:
        return notFound;
    case METHOD_NOT_ALLOWED:
        return methodNotAllowed;
    case NOT_ACCEPTABLE:
        return notAcceptable;
    case PROXY_AUTHENTICATION_REQUIRED:
        return proxyAuthenticationRequired;
    case REQUEST_TIMEOUT:
        return requestTimeout;
    case CONFLICT:
        return conflict;
    case GONE:
        return gone;
    case LENGTH_REQUIRED:
        return lengthRequired;
    case PRECONDITION_FAILED:
        return preconditionFailed;
    case PAYLOAD_TOO_LARGE:
        return payloadTooLarge;
    case URI_TOO_LONG:
        return uriTooLong;
    case UNSUPPORTED_MEDIA_TYPE:
        return unsupportedMediaType;
    case RANGE_NOT_SATISFIABLE:
        return rangeNotSatisfiable;
    case EXPECTATION_FAILED:
        return expectationFailed;
    case IM_A_TEAPOT:
        return imATeapot;
    case MISDIRECTED_REQUEST:
        return misdirectedRequest;
    case UNPROCESSABLE_ENTITY:
        return unprocessableEntity;
    case LOCKED:
        return locked;
    case FAILED_DEPENDENCY:
        return failedDependency;
    case TOO_EARLY:
        return tooEarly;
    case UPGRADE_REQUIRED:
        return upgradeRequired;
    case PRECONDITION_REQUIRED:
        return preconditionRequired;
    case TOO_MANY_REQUESTS:
        return tooManyRequests;
    case REQUEST_HEADER_FIELDS_TOO_LARGE:
        return requestHeaderFieldsTooLarge;
    case UNAVAILABLE_FOR_LEGAL_REASONS:
        return unavailableForLegalReasons;
    case INTERNAL_SERVER_ERROR:
        return internalServerError;
    case NOT_IMPLEMENTED:
        return notImplemented;
    case BAD_GATEWAY:
        return badGateway;
    case SERVICE_UNAVAILABLE:
        return serviceUnavailable;
    case GATEWAY_TIMEOUT:
        return gatewayTimeout;
    case HTTP_VERSION_NOT_SUPPORTED:
        return httpVersionNotSupported;
    case VARIANT_ALSO_NEGOTIATES:
        return variantAlsoNegotiates;
    case INSUFFICIENT_STORAGE:
        return insufficientStorage;
    case LOOP_DETECTED:
        return loopDetected;
    case NOT_EXTENDED:
        return notExtended;
    case NETWORK_AUTHENTICATION_REQUIRED:
        return networkAuthenticationRequired;
    default:
        return "Unknown status";
    }
}

HttpStatus strnToStatus(const char *str, int n)
{
#define IS_STATUS(status)                          \
    if (strncmp(statusToStr(status), str, n) == 0) \
    return (status)
    IS_STATUS(OK);
    IS_STATUS(BAD_REQUEST);
    IS_STATUS(NOT_FOUND);
    IS_STATUS(INTERNAL_SERVER_ERROR);
    return STATUS_UNKNOWN;
#undef IS_STATUS
}

HttpResp newResp(HttpStatus status)
{
    return (HttpResp){
        .version = NULL,
        .status = status,
        .headers = emptyHeaders(),
        .content = NULL,
        .contentLength = 0,
    };
}

/* returns size of string */
size_t buildRespStringFirstLineStr(HttpResp *resp, char *str, size_t size)
{
    const char *version = resp->version;
    if (version == NULL)
    {
        version = "HTTP/1.1";
    }
    return snprintf(str, size, "%s %d %s\r\n", version, resp->status, statusToStr(resp->status));
}

/* returns size of string */
size_t buildRespStringUntilContent(HttpResp *resp, char **str)
{
    ARRAY_T(char) buf = ARRAY_WITH_CAPACITY(char, 1024);
    size_t bytes = buildRespStringFirstLineStr(resp, buf.data, 1024);
    for (int i = 0; i < resp->headers.count; i++)
    {
        ARRAY_ENSURE_CAPACITY(char, &buf, buf.length + resp->headers.arr[i].key.length + resp->headers.arr[i].value.length + 4 + 2);
        bytes += sprintf(buf.data + bytes, "%s: %s\r\n", resp->headers.arr[i].key.ptr, resp->headers.arr[i].value.ptr);
    }
    bytes += sprintf(buf.data + bytes, "\r\n");
    *str = buf.data;
    return bytes;
}

void respBuilderSetVersion(HttpRespBuilder *builder, const char *version, int shouldCopy)
{
    if (shouldCopy) {
        char *copiedVersion = gcArenaAllocate(strlen(version) + 1, alignof(char));
        strcpy(copiedVersion, version);
        builder->resp.version = copiedVersion;
    } else {
        builder->resp.version = version;
    }
}

void respBuilderSetStatus(HttpRespBuilder *builder, HttpStatus status)
{
    builder->resp.status = status;
}

void verifyHeader(HttpHeader header)
{
    if (header.key.length == 0 || header.key.ptr == NULL || header.key.ptr[0] == '\0')
    {
        fprintf(stderr, "Code Error: Header key is NULL or EMPTY\n");
        exit(1);
    }
    if (header.value.length == 0 || header.value.ptr == NULL || header.value.ptr[0] == '\0')
    {
        fprintf(stderr, "Code Error: Header value is NULL or EMPTY\n");
        exit(1);
    }
    if (header.key.length > HEADER_KEY_SIZE_LIMIT) {
        fprintf(stderr, "Header key exceeds HEADER_KEY_SIZE_LIMIT=%d.\n", HEADER_KEY_SIZE_LIMIT);
        exit(1);
    }
    if (header.value.length > HEADER_VALUE_SIZE_LIMIT) {
        fprintf(stderr, "Header value exceeds HEADER_VALUE_SIZE_LIMIT=%d.\n", HEADER_VALUE_SIZE_LIMIT);
        exit(1);
    }
    for (int i = 0; i < header.key.length; i++)
    {
        char c = header.key.ptr[i];
        if (!isAlpha(c) && c != '-')
        {
            fprintf(stderr, "Header key contains non-alphabetic characters\n");
            exit(1);
        }
    }
    for (int i = 0; i < header.value.length; i++)
    {
        char c = header.value.ptr[i];
        if (c == '\r' || c == '\n' || c < 32)
        {
            fprintf(stderr, "Header value contains invalid characters\n");
            exit(1);
        }
    }

#define CANT_HAVE_HEADER(string)                                                  \
    if (strcasecmp(string, header.key.ptr) == 0)                                  \
    {                                                                             \
        fprintf(stderr, "Header key " string " is reserved and cannot be set\n"); \
        exit(1);                                                                  \
    }

    CANT_HAVE_HEADER("Content-Length");

#undef CANT_HAVE_HEADER
}

void addHeader(HttpRespBuilder *builder, const char *key, const char *value)
{
    HttpHeaders *headers = &builder->resp.headers;

    if (findHeader(headers, key) != NULL) {
        return;
    }

    int *capacity = &builder->headersCapacity;
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = gcAllocate(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = gcReallocate(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    string keyCopy = {
        .ptr = gcArenaAllocate(strlen(key) + 1, alignof(char)),
        .length = strlen(key)
    };
    strcpy(keyCopy.ptr, key);

    string valueCopy = {
        .ptr = gcArenaAllocate(strlen(value) + 1, alignof(char)),
        .length = strlen(value)
    };
    strcpy(valueCopy.ptr, value);

    HttpHeader header = {keyCopy, valueCopy};

    headers->arr[headers->count++] = header;
}

void respBuilderAddHeader(HttpRespBuilder *builder, char *key, char *value)
{
    string keyString = {.ptr = key, .length = strlen(key)};
    string valueString = {.ptr = value, .length = strlen(value)};

    verifyHeader((HttpHeader){keyString, valueString});

    string keyCopy = copyString(keyString);
    string valueCopy = copyString(valueString);
    HttpHeader header = {keyCopy, valueCopy};

    HttpHeaders *headers = &builder->resp.headers;

    int *capacity = &builder->headersCapacity;
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = gcAllocate(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = gcReallocate(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    headers->arr[headers->count++] = header;
}

void respBuilderSetContent(HttpRespBuilder *builder, const void *content, size_t contentLength, int shouldCopy)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");

    if (shouldCopy) {
        builder->resp.content = gcArenaAllocate(contentLength, alignof(char));
        memcpy(builder->resp.content, content, contentLength);
    } else {
        builder->resp.content = content;
    }
    builder->resp.contentLength = contentLength;
}

/* Read only files please */
void respBuilderSetFileContent(HttpRespBuilder *builder, const char *filePath, int shouldCopy)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");
    struct stat st;
    if (stat(filePath, &st))
    {
        if (errno == ENOENT)
        {
            respBuilderSetStatus(builder, SERVICE_UNAVAILABLE);
        }
        else
        {
            respBuilderSetStatus(builder, INTERNAL_SERVER_ERROR);
        }
    }
    else
    {
        size_t size = st.st_size;
        if (shouldCopy) {
            size_t pathSize = strlen(filePath);
            char *content = gcArenaAllocate(pathSize + 1, 1);
            snprintf(content, pathSize + 1, "%.*s", (int) pathSize, filePath);
            builder->resp.content = content;
        } else {
            builder->resp.content = filePath;
        }
        builder->resp.isContentFile = 1;
        builder->resp.contentLength = size;
    }
}

void respBuilderSetFlags(HttpRespBuilder *builder, unsigned int flags, int behaviour) {
    switch (behaviour) {
        case SET_FLAGS:
            builder->flags |= flags;
            break;
        case UNSET_FLAGS:
            builder->flags &= ~flags;
            break;
        case REPLACE_FLAGS:
            builder->flags = flags;
            break;
        default:
            fprintf(stderr, "Unknown behaviour used inside respBuilderSetFlags");
    }
}

static bool hasFlagsSet(HttpRespBuilder *builder, unsigned int flags) {
    return builder->flags & flags;
}

static const char *determineContentType(HttpRespBuilder *builder) {
    if (builder->resp.isContentFile) {
        const char *extension = getExtension(builder->resp.content);
        const char *mimeType = getMimeType(extension);
        return mimeType;
    }
    if (builder->resp.contentLength > 0) {
        return defaultMimeType;
    }
    return NULL;
}

static void setRespStatus(HttpRespBuilder *builder) {
    if (builder->resp.status != STATUS_UNKNOWN)
    {
        return;
    }
    if (builder->resp.contentLength == 0 && hasFlagsSet(builder, USE_NO_CONTENT_RESPONSE_FLAG)) {
        builder->resp.status = NO_CONTENT;
        return;
    }
    builder->resp.status = OK;
}

HttpResp respBuild(HttpRespBuilder *builder)
{
    const size_t contentLength = builder->resp.contentLength;
    if (contentLength > 0 || !hasFlagsSet(builder, USE_NO_CONTENT_RESPONSE_FLAG))
    {
        char contentLengthStr[16];
        snprintf(contentLengthStr, 16, "%zu", contentLength);
        addHeader(builder, "Content-Length", contentLengthStr);
        const char *contentType = determineContentType(builder);
        if (contentType != NULL) {
            addHeader(builder, "Content-Type", contentType);
        }
    }
    if (hasFlagsSet(builder, USE_DEFAULT_SERVER_HEADER_FLAG)) {
        addHeader(builder, "Server", "http-server-c");
    }
    setRespStatus(builder);
    return builder->resp;
}

static unsigned int defaultRespBuilderFlags = USE_DEFAULT_SERVER_HEADER_FLAG;
HttpRespBuilder newRespBuilder() {
    HttpRespBuilder builder = {
        .resp = {
            .version = NULL,
            .status = STATUS_UNKNOWN,
            .headers = (HttpHeaders){NULL, 0},
            .content = NULL,
            .contentLength = 0,
            .isContentFile = 0,
        },
        .headersCapacity = 0,
        .flags = defaultRespBuilderFlags,
    };
    return builder;
}

void respBuilderSetDefaultFlags(const unsigned int flags) {
    defaultRespBuilderFlags = flags;
}
