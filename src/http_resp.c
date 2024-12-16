#include "http_resp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

void blockBuilder(HttpRespBuilder *builder);
HttpRespBuilder *nopSetVersion(HttpRespBuilder *builder, const char *version);
HttpRespBuilder *nopSetStatus(HttpRespBuilder *builder, HttpStatus status);
HttpRespBuilder *nopAddHeader(HttpRespBuilder *builder, char *key, char *value);
HttpRespBuilder *nopSetContent(HttpRespBuilder *builder, void *content, int length);
HttpRespBuilder *nopSetFileContent(HttpRespBuilder *builder, const char *filePath);

const char *statusToStr(HttpStatus status)
{
    static const char ok[] = "OK";
    static const char badRequest[] = "Bad Request";
    static const char notFound[] = "Not Found";
    static const char internalServerError[] = "Internal Server Error";

    switch (status)
    {
    case OK:
        return ok;
    case BAD_REQUEST:
        return badRequest;
    case NOT_FOUND:
        return notFound;
    case INTERNAL_SERVER_ERROR:
        return internalServerError;
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

int respToStr(HttpResp resp, char *str, int size)
{
    if (resp.status == STATUS_UNKNOWN)
    {
        return -1;
    }
    int bytes = 0;
    bytes += respFirstLineStr(&resp, str + bytes, size - bytes);
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

int respFirstLineStr(HttpResp *resp, char *str, int size)
{
    return snprintf(str, size, "%s %d %s\r\n", resp->version, resp->status, statusToStr(resp->status));
}

int respUntilEmptyLineStr(HttpResp *resp, char *str, int size)
{
    int bytes = 0;
    bytes += respFirstLineStr(resp, str, size);
    for (int i = 0; i < resp->headers.count; i++)
    {
        bytes += snprintf(str + bytes, size - bytes, "%s: %s\r\n", resp->headers.arr[i].key, resp->headers.arr[i].value);
    }
    bytes += snprintf(str + bytes, size - bytes, "\r\n");
    return bytes;
}

HttpRespBuilder *setVersion(HttpRespBuilder *builder, const char *version)
{
    builder->resp.version = malloc(strlen(version) + 1);
    strcpy(builder->resp.version, version);
    return builder;
}

HttpRespBuilder *setStatus(HttpRespBuilder *builder, HttpStatus status)
{
    builder->resp.status = status;
    return builder;
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

    char *keyCopy = malloc(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = malloc(strlen(value) + 1);
    strcpy(valueCopy, value);

    for (int i = 0; i < headers->count; i++)
    {
        if (strcasecmp(headers->arr[i].key, key) == 0)
        {
            free(headers->arr[i].key);
            free(headers->arr[i].value);
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
        headers->arr = malloc(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = realloc(headers->arr, sizeof(HttpHeader) * (*capacity));
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
        headers->arr = malloc(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = realloc(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    char *keyCopy = malloc(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = malloc(strlen(value) + 1);
    strcpy(valueCopy, value);
    HttpHeader header = {keyCopy, valueCopy};

    headers->arr[headers->count++] = header;
}

HttpRespBuilder *addHeaderVerify(HttpRespBuilder *builder, char *key, char *value)
{
    verifyHeader((HttpHeader){key, value});

    char *keyCopy = malloc(strlen(key) + 1);
    strcpy(keyCopy, key);
    char *valueCopy = malloc(strlen(value) + 1);
    strcpy(valueCopy, value);
    HttpHeader header = {keyCopy, valueCopy};
    HttpHeaders *headers = &builder->resp.headers;
    int *capacity = &builder->headersCapacity;
    if (headers->count == 0)
    {
        *capacity = 1;
        headers->arr = malloc(sizeof(HttpHeader) * (*capacity));
    }
    else if (headers->count >= *capacity)
    {
        *capacity *= 2;
        headers->arr = realloc(headers->arr, sizeof(HttpHeader) * (*capacity));
    }

    headers->arr[headers->count++] = header;
    return builder;
}

HttpRespBuilder *setContent(HttpRespBuilder *builder, void *content, int contentLength)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");

    builder->resp.content = malloc(contentLength);
    memcpy(builder->resp.content, content, contentLength);
    builder->resp.contentLength = contentLength;
    return builder;
}

/* Read only files please */
HttpRespBuilder *setFileContent(HttpRespBuilder *builder, const char *filePath)
{
    assert(builder->resp.content == NULL && "The builder already has some content set");
    FILE *file = fopen(filePath, "r");
    if (file == NULL)
    {
        if (errno = ENOENT) {
            builder->setStatus(builder, NOT_FOUND);
        } else {
            builder->setStatus(builder, INTERNAL_SERVER_ERROR);
        }
        freeHeaders(&builder->resp.headers);
    }
    else
    {
        struct stat st;
        stat(filePath, &st);
        int size = st.st_size;
        char *content = malloc(size);
        fread(content, 1, size, file);
        fclose(file);
        builder->resp.content = content;
        builder->resp.contentLength = size;
    }

    return builder;
}

HttpResp build(HttpRespBuilder *builder)
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
        builder->addHeader(builder, "Server", "http-server-c");
    }
    if (builder->resp.status == STATUS_UNKNOWN)
    {
        builder->resp.status = OK;
    }
    return builder->resp;
}

HttpRespBuilder newRespBuilder()
{
    char http11[] = "HTTP/1.1";
    char *version = malloc(sizeof(http11));
    strcpy(version, http11);
    HttpRespBuilder builder = {
        .resp = {
            .version = version,
            .status = STATUS_UNKNOWN,
            .headers = (HttpHeaders){NULL, 0},
            .content = NULL,
            .contentLength = 0,
        },
        .setVersion = setVersion,
        .setStatus = setStatus,
        .addHeader = addHeaderVerify,
        .setContent = setContent,
        .setFileContent = setFileContent,
        .build = build,
        .headersCapacity = 0,
    };
    return builder;
}

void blockBuilder(HttpRespBuilder *builder)
{
    builder->setVersion = nopSetVersion;
    builder->setStatus = nopSetStatus;
    builder->addHeader = nopAddHeader;
    builder->setContent = nopSetContent;
    builder->setFileContent = nopSetFileContent;
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
    free(resp->version);
    freeHeaders(&resp->headers);
    free(resp->content);
}

HttpRespBuilder *nopSetVersion(HttpRespBuilder *builder, const char *version)
{
    return builder;
}

HttpRespBuilder *nopSetStatus(HttpRespBuilder *builder, HttpStatus status)
{
    return builder;
}

HttpRespBuilder *nopAddHeader(HttpRespBuilder *builder, char *key, char *value)
{
    return builder;
}

HttpRespBuilder *nopSetContent(HttpRespBuilder *builder, void *content, int length)
{
    return builder;
}

HttpRespBuilder *nopSetFileContent(HttpRespBuilder *builder, const char *filePath)
{
    return builder;
}