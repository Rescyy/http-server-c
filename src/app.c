#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "app.h"
#include "connection.h"
#include "http_router.h"
#include "utils.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static HttpRouter router = {.capacity = -1};
static const char *logFilePath = NULL;
void *handleConnectionThreadCall(void *arg);
void handleConnection(TcpSocket client);
void sendResponse(HttpResp *resp, HttpReq *req, char *stackBuffer, TcpSocket *client);
int sendAll(TcpSocket *client, void *buffer, int size);
int recvAll(TcpSocket *client, void *buffer, int size);
void logResponse(HttpReq *req, HttpResp *resp, TcpSocket *client);
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
#define COLOR_RESET "\e[m\033[0m"
#define COLOR_COUNT sizeof(colorCodes) / sizeof(*colorCodes)
#define THREAD_NAME_FORMAT "%sThread %ld " COLOR_RESET
#define THREAD_NAME_ARGS(threadId) colorCodes[hash(&threadId, sizeof(long)) % COLOR_COUNT], threadId

void setLogFile(const char *path)
{
    if (access(path, F_OK) != 0)
    {
        FILE *file = fopen(path, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Failed to open log file\n");
            return;
        }
        fclose(file);
    }
    logFilePath = path;
}

void startApp(char *port)
{

    signal(SIGPIPE, SIG_IGN);

    if (router.capacity == -1)
    {
        router = emptyRouter();
    }
    TcpSocket socket = socketListen(port);
    pthread_t thread1;

    if (socket.error == -1)
    {
        fprintf(stderr, "Socket port unavailable\n");
        exit(1);
    }
    else if (socket.error != 0)
    {
        fprintf(stderr, "Socket listen failed: %s\n", strerror(socket.error));
        exit(1);
    }
    else
    {
        printf("Listening to port %s.\n", port);
    }

    for (;;)
    {
        TcpSocket *client = (TcpSocket *)malloc(sizeof(TcpSocket));
        *client = acceptConnection(socket);
        if (hasError(*client))
        {
            fprintf(stderr, "Client connection error: %s\n", strerror(client->error));
        }
        else
        {
            pthread_create(&thread1, NULL, handleConnectionThreadCall, (void *)client);
            pthread_detach(thread1);
        }
    }
}

void *handleConnectionThreadCall(void *arg)
{
    TcpSocket client = *((TcpSocket *)arg);
    handleConnection(client);

    closeSocket(client);
    free(arg);
    pthread_exit(NULL);
}

#define BUFFER_SIZE 1024

void handleConnection(TcpSocket client)
{
    char buffer[BUFFER_SIZE];

    int bytesReceived = recv(client.fd, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == -1)
    {
        long threadId = (long)pthread_self();
        fprintf(stderr, THREAD_NAME_FORMAT "%-16s %s\n", THREAD_NAME_ARGS(threadId), client.ip, strerror(errno));
        return;
    }

    HttpReq request;
    int additionalBytes = parseRequest(&request, buffer, bytesReceived);
    if (additionalBytes == -1)
    {
        char badRequestResponse[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
        logResponse(&request, &(HttpResp){.status = 400}, &client);
        send(client.fd, badRequestResponse, sizeof(badRequestResponse) - 1, 0);
        return;
    }
    else if (additionalBytes > 0)
    {
        void *additionalBuffer = malloc(additionalBytes);
        recvAll(&client, additionalBuffer, additionalBytes);
        addAdditionalRequestContent(&request, additionalBuffer, additionalBytes);
        free(additionalBuffer);
    }
    HttpResp resp = routeReq(router, request);
    logResponse(&request, &resp, &client);
    sendResponse(&resp, &request, buffer, &client);
    freeResp(&resp);
    freeReq(&request);
}

void sendResponse(HttpResp *resp, HttpReq *req, char *stackBuffer, TcpSocket *client)
{
    int responseSize = respUntilEmptyLineStr(resp, stackBuffer, BUFFER_SIZE);
    if (responseSize > BUFFER_SIZE)
    {
        char *responseBuffer = malloc(responseSize);
        respUntilEmptyLineStr(resp, responseBuffer, responseSize);
        int result = sendAll(client, responseBuffer, responseSize);
        if (result == -1)
        {
            free(responseBuffer);
            return;
        }
        if (resp->contentLength > 0)
        {
            result = sendAll(client, resp->content, resp->contentLength);
            if (result == -1)
            {
                free(responseBuffer);
                return;
            }
        }
        free(responseBuffer);
    }
    else
    {
        int result = sendAll(client, stackBuffer, responseSize);
        if (result == -1)
        {
            return;
        }
        if (resp->contentLength > 0)
        {
            result = sendAll(client, resp->content, resp->contentLength);
            if (result == -1)
            {
                return;
            }
        }
    }
}

int sendAll(TcpSocket *client, void *buffer, int size)
{
    int sent = 0;
    while (sent < size)
    {
        int packetSize = send(client->fd, buffer + sent, MIN(size - sent, 1 << 20), 0);
        if (packetSize == -1)
        {
            long threadId = (long)pthread_self();
            fprintf(stderr, THREAD_NAME_FORMAT "%-16s %s\n", THREAD_NAME_ARGS(threadId), client->ip, strerror(errno));
            return -1;
        }
        sent += packetSize;
        // printf("Sent %d bytes. Total bytes %d/%d\n", packetSize, sent , size);
    }
    return 0;
}

int recvAll(TcpSocket *client, void *buffer, int size)
{
    int received = 0;
    while (received < size)
    {
        int packetSize = recv(client->fd, buffer + received, size - received, 0);
        if (packetSize == -1)
        {
            long threadId = (long)pthread_self();
            fprintf(stderr, THREAD_NAME_FORMAT "%s\n", THREAD_NAME_ARGS(threadId), strerror(errno));
            return -1;
        }
        received += packetSize;
    }
    return 0;
}

void logResponse(HttpReq *req, HttpResp *resp, TcpSocket *client)
{
    char path[1024];
    pathToStr(path, 1024, req->path);
    long threadId = (long)pthread_self();
    FILE *file = NULL;
    if (logFilePath != NULL)
    {
        FILE *file = fopen(logFilePath, "a");
        if (file == NULL)
        {
            return;
        }
        fprintf(file, "Thread %ld %-16s %-5s %-50s | %d %s\n", threadId, client->ip, methodToStr(req->method), path, resp->status, statusToStr(resp->status));
        fclose(file);
    }
    else
    {
        printf(THREAD_NAME_FORMAT "%-16s %-5s %-50s | %d %s\n", THREAD_NAME_ARGS(threadId), client->ip, methodToStr(req->method), path, resp->status, statusToStr(resp->status));
    }
}

void addEndpoint(char *path, HttpReqHandler handler)
{
    if (router.capacity == -1)
    {
        router = emptyRouter();
    }
    HttpEndpoint endpoint = newEndpoint(path, handler);
    routerAddEndpoint(&router, endpoint);
}

void setNotFoundCallback(HttpReqHandler handler)
{
    router.notFoundCallback = handler;
}