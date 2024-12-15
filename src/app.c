#include "app.h"
#include "http_router.h"
#include "connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static HttpRouter router = {.capacity = -1};
FILE *logFile;
void *handleConnectionThreadCall(void *arg);
void handleConnection(TcpSocket client);
void sendResponse(HttpResp resp, HttpReq *req, char *stackBuffer, TcpSocket *client);
int sendAll(TcpSocket *client, void *buffer, int size);
int recvAll(TcpSocket *client, void *buffer, int size);
void logResponse(FILE *file, HttpReq *req, HttpResp *resp, TcpSocket *client);
static char *colorCodes[] = {
    "\x1B[30m",
    "\x1B[31m",
    "\x1B[32m",
    "\x1B[33m",
    "\x1B[34m",
    "\x1B[35m",
    "\x1B[36m",
};
#define COLOR_RESET "\033[0m"
#define COLOR_COUNT sizeof(colorCodes) / sizeof(char *)
#define THREAD_NAME_FORMAT "%sThread %ld " COLOR_RESET
#define THREAD_NAME_ARGS(threadID) colorCodes[hash(&threadID, sizeof(long)) % COLOR_COUNT], threadID

void startApp(char *port)
{
    logFile = stdout;

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
        long threadID = (long)pthread_self();
        fprintf(stderr, THREAD_NAME_FORMAT"%-16s %s\n", THREAD_NAME_ARGS(threadID), client.ip, strerror(errno));
        return;
    }

    HttpReq request;
    int additionalBytes = parseRequest(&request, buffer, bytesReceived);
    if (additionalBytes == -1)
    {
        char badRequestResponse[] = "HTTP/1.1 400 Bad Request\r\n\r\n";
        HttpRespBuilder builder = newRespBuilder();
        logResponse(logFile, &request, &(HttpResp){.status = 400}, &client);
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
    logResponse(logFile, &request, &resp, &client);
    sendResponse(resp, &request, buffer, &client);
    freeResp(&resp);
    freeReq(&request);
}

/*
    Frees response and request after sending.
*/
void sendResponse(HttpResp resp, HttpReq *req, char *stackBuffer, TcpSocket *client)
{
    int responseSize = respUntilEmptyLineStr(&resp, stackBuffer, BUFFER_SIZE);
    if (responseSize > BUFFER_SIZE)
    {
        char *responseBuffer = malloc(responseSize);
        int ssss = respUntilEmptyLineStr(&resp, responseBuffer, responseSize);
        int result = send(client->fd, responseBuffer, responseSize, 0);
        if (result == -1)
        {
            free(responseBuffer);
            return;
        }
        if (resp.contentLength > 0)
        {
            result = sendAll(client, resp.content, resp.contentLength);
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
        if (resp.contentLength > 0)
        {
            result = sendAll(client, resp.content, resp.contentLength);
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
            long threadID = (long)pthread_self();
            fprintf(stderr, THREAD_NAME_FORMAT"%-16s %s\n", THREAD_NAME_ARGS(threadID), client->ip, strerror(errno));
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
            long threadID = (long)pthread_self();
            fprintf(stderr, THREAD_NAME_FORMAT"%s\n", THREAD_NAME_ARGS(threadID), strerror(errno));
            return -1;
        }
        received += packetSize;
    }
    return 0;
}

void logResponse(FILE *file, HttpReq *req, HttpResp *resp, TcpSocket *client)
{
    char path[1024];
    pathToStr(path, 1024, req->path);
    char respFirstLine[1024];
    respFirstLineStr(resp, respFirstLine, 1024);
    long threadID = (long)pthread_self();
    fprintf(file, THREAD_NAME_FORMAT"%-16s Response to %-35s | %s", THREAD_NAME_ARGS(threadID), client->ip, path, respFirstLine);
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