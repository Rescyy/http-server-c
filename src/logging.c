//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#include "../includes/logging.h"
#include "../includes/utils.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>

#define COLOR_RESET "\e[m\033[0m"
#define COLOR_COUNT sizeof(colorCodes) / sizeof(*colorCodes)
#define CONNECTION_NAME_FORMAT "Request %lu.%lu"
#define THREAD_NAME_FORMAT "%sThread %lu" COLOR_RESET
#define THREAD_NAME_ARGS(threadId) colorCodes[hash((void *)threadId, sizeof(long)) % COLOR_COUNT], threadId

static char *colorCodes[] = {
    "\x1B[30m",
    "\x1B[31m",
    "\x1B[32m",
    "\x1B[33m",
    "\x1B[34m",
    "\x1B[35m",
    "\x1B[36m",
    "\e[1m\x1B[30m",
    "\e[1m\x1B[31m",
    "\e[1m\x1B[32m",
    "\e[1m\x1B[33m",
    "\e[1m\x1B[34m",
    "\e[1m\x1B[35m",
    "\e[1m\x1B[36m",
};
static const char *logFilePath = NULL;
static const char *jsonLogFilePath = NULL;
static int logFlags = PRINT_LOG;

static const char *logLevelToStr(LogLevel logLevel) {
    switch (logLevel) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        case FATAL:
            return "FATAL";
        case TRACE_:
            return "TRACE";
        default:
            return "UNKNOWN LOG LEVEL";
    }
}

static void _log(LogLevel logLevel, const char *format, va_list args) {
    fflush(stdout);
    DECLARE_CURRENT_TIME(time);
    SessionState *state = getSessionState();
    if (state == NULL) {
        printf("[%s|%s|MAIN] ", logLevelToStr(logLevel), time);
    } else {
        printf("[%s|%s|%lu.%lu] ", logLevelToStr(logLevel), time, state->connectionIndex, state->requestIndex);
    }
    vprintf(format, args);
    putchar('\n');
    fflush(stdout);
}

void trace(const char *file, int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    DECLARE_CURRENT_TIME(time);
    SessionState *state = getSessionState();
    if (state == NULL) {
        printf("[%s|%s|MAIN|%s:%d] ", logLevelToStr(TRACE_), time, file, line);
    } else {
        printf("[%s|%s|%lu.%lu|%s:%d] ", logLevelToStr(TRACE_), time, state->connectionIndex, state->requestIndex, file, line);
    }
    vprintf(format, args);
    putchar('\n');
    va_end(args);
}

void debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log(DEBUG, format, args);
    va_end(args);
}

void info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log(INFO, format, args);
    va_end(args);
}

void warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log(WARNING, format, args);
    va_end(args);
}

void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log(ERROR, format, args);
    va_end(args);
}

void fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);
    _log(FATAL, format, args);
    va_end(args);
}

/*
 * Sets the logging file path, sets the FILE_LOG log flag
 */
void setLogFile(const char *path)
{
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open log file %s\n", path);
        return;
    }
    fclose(file);
    logFilePath = path;
    logFlags |= FILE_LOG;
}

void setJsonLogFile(const char *path) {
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open log file %s\n", path);
        return;
    }
    fclose(file);
    jsonLogFilePath = path;
    logFlags |= JSON_LOG;
}

void setSocketLogFile(const char *path) {

}

void setLogFlags(int flags)
{
    logFlags = flags;
}

void logResponse(HttpResp *resp, HttpReq *req)
{
    unsigned long threadId = pthread_self();
    char *clientIp = req->appState->clientSocket.ip;
    unsigned long connectionIndex = req->appState->connectionIndex;
    unsigned long requestIndex = req->appState->requestIndex;
    if (logFlags & PRINT_LOG)
    {
        printf(CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %-7s %-50s | %d %s\n",
            connectionIndex,
            requestIndex,
            THREAD_NAME_ARGS(threadId),
            clientIp,
            methodToStr(req->method),
            req->path.raw,
            resp->status,
            statusToStr(resp->status)
        );
    }
    if (logFlags & JSON_LOG && jsonLogFilePath != NULL) {
        FILE *file = fopen(jsonLogFilePath, "a");
        if (file == NULL) {
            return;
        }
        char *buffer;

        DECLARE_CURRENT_TIME(currentTime);

        JObject reqObj = httpReqToJObject(req);
        JToken reqToken = toJToken_JObject(reqObj);
        JToken timestampToken = toJToken_long(time(NULL));
        JToken formattedTimeToken = toJToken_cstring(currentTime);
        JToken clientIpToken = toJToken_cstring(clientIp);

        JObject logObj = _JObject(
            _JProperty("request", reqToken),
            _JProperty("timestamp", timestampToken),
            _JProperty("formattedTime", formattedTimeToken),
            _JProperty("clientIp", clientIpToken)
        );
        JToken logToken = toJToken_JObject(logObj);

        unsigned int size = serializeJson(logToken, &buffer, 4);
        fwrite(buffer, size, 1, file);
        char entrySeparator[] = ",\n";
        fwrite(entrySeparator, strlen(entrySeparator), 1, file);
        fclose(file);
    }
    if (logFlags & FILE_LOG && logFilePath != NULL) {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL) {
            return;
        }
        fprintf(file, CONNECTION_NAME_FORMAT " Thread %ld %-16s %-7s %-50s | %d %s\n",
            connectionIndex,
            requestIndex,
            threadId,
            clientIp,
            methodToStr(req->method),
            req->path.raw,
            resp->status,
            statusToStr(resp->status)
        );
        fclose(file);
    }
}

void logErrorResponse(HttpReq *req, const char *error)
{
    char path[1024];
    strncpy(path, req->path.raw, 1024);
    unsigned long threadId = pthread_self();
    char *clientIp = req->appState->clientSocket.ip;
    unsigned long connectionIndex = req->appState->connectionIndex;
    unsigned long requestIndex = req->appState->requestIndex;
    if (logFlags & PRINT_LOG)
    {
        printf(CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %-7s %-50s | Error: %s\n", connectionIndex, requestIndex, THREAD_NAME_ARGS(threadId), clientIp, methodToStr(req->method), path, error);
    }
    if (logFlags & FILE_LOG)
    {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL)
        {
            return;
        }
        fprintf(file, CONNECTION_NAME_FORMAT " Thread %ld %-16s %-7s %-50s | Error: %s\n", connectionIndex, requestIndex, threadId, clientIp, methodToStr(req->method), path, error);
        fclose(file);
    }
}

void printError(HttpReq *req, TcpSocket *client) {
    unsigned long threadId = pthread_self();
    unsigned long connectionIndex = req->appState->connectionIndex;
    unsigned long requestIndex = req->appState->requestIndex;
    fprintf(stderr, CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %s\n", connectionIndex, requestIndex, THREAD_NAME_ARGS(threadId), client->ip, strerror(errno));
}