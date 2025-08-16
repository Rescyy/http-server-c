//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "http_resp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "alloc.h"
#include "utils.h"
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

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
int respToStr(HttpResp resp, char *str, int size)
{
    if (resp.status == STATUS_UNKNOWN)
    {
        return -1;
    }
    int bytes = 0;
    bytes += buildRespStringFirstLineStr(&resp, str + bytes, size - bytes);
    for (int i = 0; i < resp.headers.count; i++)
    {
        bytes += snprintf(str + bytes, size - bytes, "%s: %s\r\n", resp.headers.arr[i].key, resp.headers.arr[i].value);
    }
    bytes += snprintf(str + bytes, size - bytes, "\r\n");

    if (size - bytes < resp.contentLength)
    {
        memcpy(str + bytes, resp.content, size - bytes);
    }
    else
    {
        memcpy(str + bytes, resp.content, resp.contentLength);
    }
    return bytes + resp.contentLength;
}

/* returns size of string */
int buildRespStringFirstLineStr(HttpResp *resp, char *str, int size)
{
    char *version = resp->version;
    if (version == NULL)
    {
        version = "HTTP/1.1";
    }
    return snprintf(str, size, "%s %d %s\r\n", version, resp->status, statusToStr(resp->status));
}

/* returns size of string */
int buildRespStringUntilContent(HttpResp *resp, char *str, int size)
{
    int bytes = 0;
    bytes += buildRespStringFirstLineStr(resp, str, size);
    for (int i = 0; i < resp->headers.count; i++)
    {
        bytes += snprintf(str + bytes, size - bytes, "%s: %s\r\n", resp->headers.arr[i].key, resp->headers.arr[i].value);
    }
    bytes += snprintf(str + bytes, size - bytes, "\r\n");
    return bytes;
}

void respBuilderSetVersion(HttpRespBuilder *builder, const char *version)
{
    builder->resp.version = allocate(strlen(version) + 1);
    strcpy(builder->resp.version, version);
}

void respBuilderSetStatus(HttpRespBuilder *builder, HttpStatus status)
{
    builder->resp.status = status;
}

void verifyHeader(HttpHeader header)
{
    if (header.key == NULL || header.key[0] == '\0')
    {
        fprintf(stderr, "Header key is NULL\n");
        exit(1);
    }
    if (header.value == NULL || header.value[0] == '\0')
    {
        fprintf(stderr, "Header value is NULL\n");
        exit(1);
    }
    for (int i = 0; header.key[i] != '\0'; i++)
    {
        char c = header.key[i];
        if (!isAlpha(c) && c != '-')
        {
            fprintf(stderr, "Header key contains non-alphabetic characters\n");
            exit(1);
        }
        if (i >= HEADER_KEY_SIZE_LIMIT)
        {
            fprintf(stderr, "Header key exceeds HEADER_KEY_SIZE_LIMIT=%d.\n", HEADER_KEY_SIZE_LIMIT);
            exit(1);
        }
    }
    for (int i = 0; header.value[i] != '\0'; i++)
    {
        char c = header.value[i];
        if (c == '\r' || c == '\n' || c < 32)
        {
            fprintf(stderr, "Header value contains invalid characters\n");
            exit(1);
        }
        if (i >= HEADER_VALUE_SIZE_LIMIT)
        {
            fprintf(stderr, "Header value exceeds HEADER_VALUE_SIZE_LIMIT=%d.\n", HEADER_VALUE_SIZE_LIMIT);
            exit(1);
        }
    }

#define CANT_HAVE_HEADER(string)                                                  \
    if (strcasecmp(string, header.key) == 0)                                      \
    {                                                                             \
        fprintf(stderr, "Header key " string " is reserved and cannot be set\n"); \
        exit(1);                                                                  \
    }

    CANT_HAVE_HEADER("Content-Length");

#undef CANT_HAVE_HEADER
}

void addHeaderImposing(HttpRespBuilder *builder, char *key, char *value)
{
    HttpHeaders *headers = &builder->resp.headers;

    char *keyCopy = allocate(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = allocate(strlen(value) + 1);
    strcpy(valueCopy, value);

    for (int i = 0; i < headers->count; i++)
    {
        if (strcasecmp(headers->arr[i].key, key) == 0)
        {
            deallocate(headers->arr[i].key);
            deallocate(headers->arr[i].value);
            headers->arr[i].key = keyCopy;
            headers->arr[i].value = valueCopy;
            return;
        }
    }

    int *capacity = &builder->headersCapacity;
    HttpHeader header = {keyCopy, valueCopy};
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = allocate(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = reallocate(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    headers->arr[headers->count++] = header;
}

void addHeader(HttpRespBuilder *builder, char *key, char *value)
{
    HttpHeaders *headers = &builder->resp.headers;

    for (int i = 0; i < headers->count; i++)
    {
        if (strcasecmp(headers->arr[i].key, key) == 0)
        {
            return;
        }
    }

    int *capacity = &builder->headersCapacity;
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = allocate(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = reallocate(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    char *keyCopy = allocate(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = allocate(strlen(value) + 1);
    strcpy(valueCopy, value);
    HttpHeader header = {keyCopy, valueCopy};

    headers->arr[headers->count++] = header;
}

void respBuilderAddHeader(HttpRespBuilder *builder, char *key, char *value)
{
    verifyHeader((HttpHeader){key, value});

    char *keyCopy = allocate(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = allocate(strlen(value) + 1);
    strcpy(valueCopy, value);
    HttpHeader header = {keyCopy, valueCopy};
    HttpHeaders *headers = &builder->resp.headers;
    int *capacity = &builder->headersCapacity;
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = allocate(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = reallocate(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    headers->arr[headers->count++] = header;
}

void respBuilderSetContent(HttpRespBuilder *builder, void *content, int contentLength)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");

    builder->resp.content = allocate(contentLength);
    memcpy(builder->resp.content, content, contentLength);
    builder->resp.contentLength = contentLength;
}

/* Read only files please */
void respBuilderSetFileContent(HttpRespBuilder *builder, const char *filePath)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");
    struct stat st;
    if (stat(filePath, &st))
    {
        if (errno == ENOENT)
        {
            respBuilderSetStatus(builder, NOT_FOUND);
        }
        else
        {
            respBuilderSetStatus(builder, INTERNAL_SERVER_ERROR);
        }
        freeHeaders(&builder->resp.headers);
    }
    else
    {
        size_t size = st.st_size;
        size_t pathSize = strlen(filePath);
        char *content = malloc(pathSize + 1);
        strncpy(content, filePath, pathSize);
        builder->resp.isContentFile = 1;
        builder->resp.content = content;
        builder->resp.contentLength = size;
    }
}

HttpResp respBuild(HttpRespBuilder *builder)
{
    int contentLength = builder->resp.contentLength;
    if (contentLength > 0)
    {
        char contentLengthStr[16];
        snprintf(contentLengthStr, 16, "%d", contentLength);
        addHeader(builder, "Content-Length", contentLengthStr);
        addHeader(builder, "Content-Type", "text/plain"); /*will not overwrite Content-Type*/
    }
    if (findHeader(builder->resp.headers, "Server") == NULL)
    {
        respBuilderAddHeader(builder, "Server", "http-server-c");
    }
    if (builder->resp.status == STATUS_UNKNOWN)
    {
        builder->resp.status = OK;
    }
    return builder->resp;
}

HttpRespBuilder newRespBuilder()
{
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
    };
    return builder;
}

int respEq(HttpResp obj1, HttpResp obj2)
{
    if (strcmp(obj1.version, obj2.version) != 0)
    {
        return 0;
    }
    if (obj1.status != obj2.status)
    {
        return 0;
    }
    if (!headersEq(obj1.headers, obj2.headers))
    {
        return 0;
    }
    if (obj1.contentLength != obj2.contentLength)
    {
        return 0;
    }
    if (memcmp(obj1.content, obj2.content, obj1.contentLength) != 0)
    {
        return 0;
    }
    return 1;
}

void freeResp(HttpResp *resp)
{
    deallocate(resp->version);
    freeHeaders(&resp->headers);
    deallocate(resp->content);
}
