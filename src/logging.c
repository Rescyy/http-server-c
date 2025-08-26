//
// Created by Crucerescu Vladislav on 13.08.2025.
//

#include "logging.h"
#include "utils.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define COLOR_RESET "\e[m\033[0m"
#define COLOR_COUNT sizeof(colorCodes) / sizeof(*colorCodes)
#define CONNECTION_NAME_FORMAT "Connection %lu"
#define THREAD_NAME_FORMAT "%sThread %ld" COLOR_RESET
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
static int logFlags = PRINT_LOG;

/*
 * Sets the logging file path, sets the FILE_LOG log flag
 */
void setLogFile(const char *path)
{
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open log file %s\n", path);
    }
    fclose(file);
    logFilePath = path;
    logFlags |= FILE_LOG;
}

void setLogFlags(int flags)
{
    if ((flags | FILE_LOG | JSON_LOG) && logFilePath == NULL) {
        fprintf(stderr, "Before setting the FILE_LOG flag, set a log file.\n");
    }
    logFlags = flags;
}

void logResponse(HttpResp *resp, HttpReq *req)
{
    char path[1024];
    strncpy(path, req->path.raw, 1024);
    long threadId = (long)pthread_self();
    char *clientIp = req->appState->clientSocket.ip;
    unsigned long connectionIndex = req->appState->connectionIndex;
    if (logFlags & PRINT_LOG)
    {
        printf(CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %-7s %-50s | %d %s\n", connectionIndex, THREAD_NAME_ARGS(threadId), clientIp, methodToStr(req->method), path, resp->status, statusToStr(resp->status));
    }
    if (logFlags & JSON_LOG) {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL) {
            return;
        }
        char *buffer;
        JObject reqObj = httpReqToJObject(req);
        JToken reqToken = _JToken(reqObj);
        unsigned int size = serializeJson(reqToken, &buffer, 4);
        fwrite(buffer, size, 1, file);
        fclose(file);
        freeHttpReqJObject(&reqObj);
    } else if (logFlags & FILE_LOG) {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL) {
            return;
        }
        fprintf(file, CONNECTION_NAME_FORMAT " Thread %ld %-16s %-7s %-50s | %d %s\n", connectionIndex, threadId,
                clientIp, methodToStr(req->method), path, resp->status, statusToStr(resp->status));
        fclose(file);
    }
}

void logError(HttpReq *req, const char *error)
{
    char path[1024];
    strncpy(path, req->path.raw, 1024);
    long threadId = (long)pthread_self();
    char *clientIp = req->appState->clientSocket.ip;
    unsigned long connectionIndex = req->appState->connectionIndex;
    if (logFlags & PRINT_LOG)
    {
        printf(CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %-7s %-50s | Error: %s\n", connectionIndex, THREAD_NAME_ARGS(threadId), clientIp, methodToStr(req->method), path, error);
    }
    if (logFlags & FILE_LOG)
    {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL)
        {
            return;
        }
        fprintf(file, CONNECTION_NAME_FORMAT " Thread %ld %-16s %-7s %-50s | Error: %s\n", connectionIndex, threadId, clientIp, methodToStr(req->method), path, error);
        fclose(file);
    }
}

void printError(HttpReq *req, TcpSocket *client) {
    long threadId = (long)pthread_self();
    unsigned long connectionIndex = req->appState->connectionIndex;
    fprintf(stderr, CONNECTION_NAME_FORMAT " " THREAD_NAME_FORMAT " %-16s %s\n", connectionIndex, THREAD_NAME_ARGS(threadId), client->ip, strerror(errno));
}