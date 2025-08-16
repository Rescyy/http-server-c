//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "alloc.h"
#include "app.h"
#include "app_state.h"
#include "connection.h"
#include "http_router.h"
#include "logging.h"
#include "tcp_stream.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static HttpRouter router = {.capacity = -1};

void *handleConnectionThreadCall(void *arg);

void handleConnection(SessionState *appState);

void sendResponse(HttpResp *resp, TcpSocket *client);

int sendAll(TcpSocket *client, void *buffer, int size);

int handleError(int result, TcpSocket *client, HttpReq *request);

int handleRequest(SessionState *appState, TcpStream *stream);

void startApp(char *port) {
    signal(SIGPIPE, SIG_IGN);

    if (router.capacity == -1) {
        router = emptyRouter();
    }
    TcpSocket socket = socketListen(port);
    pthread_t thread1;

    if (socket.error == -1) {
        fprintf(stderr, "Socket port unavailable\n");
        exit(1);
    }
    if (socket.error != 0) {
        fprintf(stderr, "Socket listen failed: %s\n", strerror(socket.error));
        exit(1);
    }
    printf("Listening to port %s.\n", port);
    int connectionIndex = 1;

    for (;;) {
        SessionState *appState = newSessionAppState();
        appState->connectionIndex = connectionIndex++;
        TcpSocket clientSocket = acceptConnection(socket);

        if (hasError(clientSocket)) {
            fprintf(stderr, "Connection Index [%lu] Client connection error: %s\n",
                    appState->connectionIndex,
                    strerror(clientSocket.error));
        } else {
            appState->clientSocket = clientSocket;
            pthread_create(&thread1, NULL, handleConnectionThreadCall, appState);
            pthread_detach(thread1);
        }
    }
}

void *handleConnectionThreadCall(void *arg) {
    start_alloc_tracking();
    SessionState *appState = (SessionState *) arg;
    handleConnection(appState);
    closeSocket(appState->clientSocket);
    stop_alloc_tracking();
    deallocate(arg);
    pthread_exit(NULL);
}

#define BUFFER_SIZE 1024

void handleConnection(SessionState *appState) {
    TcpStream stream = newTcpStream(appState->clientSocket.fd);

    while (handleRequest(appState, &stream)) {
    }

    freeTcpStream(&stream);
}

int handleRequest(SessionState *appState, TcpStream *stream) {
    HttpReq request = {
        .appState = appState
    };
    HttpResp resp;
    tcpStreamWait(stream);
    if (stream->error == TCP_STREAM_CLOSED || stream->error == TCP_STREAM_ERROR) {
        return 0;
    }
    int result = parseRequestStream(&request, stream);
    int action = handleError(result, &appState->clientSocket, &request);
    switch (action) {
        case 1:
            freeReq(&request);
            tcpStreamDrain(stream);
            return 0;
        case 2:
            freeReq(&request);
            tcpStreamDrain(stream);
            return 1;
        default:
            break;
    }

    resp = routeReq(router, request);

    logResponse(&resp, &request);
    sendResponse(&resp, &appState->clientSocket);

    freeResp(&resp);
    freeReq(&request);

    tcpStreamDrain(&stream);

    return isConnectionKeepAlive(&request);
}

/*
 * Returns 1 if should close connection.
 * Returns 2 if should respond with error.
 */
int handleError(int result, TcpSocket *client, HttpReq *request) {
    HttpResp resp;
    switch (result) {
        case TCP_STREAM_ERROR:
            logError(request, "TCP Stream error");
        case TCP_STREAM_CLOSED:
            return 1;
        case ENTITY_TOO_LARGE_ERROR:
            resp = newResp(PAYLOAD_TOO_LARGE);
            break;
        case UNKNOWN_METHOD:
            resp = newResp(METHOD_NOT_ALLOWED);
            break;
        case UNKNOWN_VERSION:
            resp = newResp(HTTP_VERSION_NOT_SUPPORTED);
            break;
        case BAD_REQUEST_ERROR:
            resp = newResp(BAD_REQUEST);
            break;
        case URI_TOO_LARGE_ERROR:
            resp = newResp(URI_TOO_LONG);
            break;
        default:
            return 0;
    }
    logResponse(&resp, request);
    sendResponse(&resp, client);
    return 2;
}

void sendResponse(HttpResp *resp, TcpSocket *client) {
    char stackBuffer[BUFFER_SIZE];
    int responseSize = respUntilEmptyLineStr(resp, stackBuffer, BUFFER_SIZE);
    if (responseSize > BUFFER_SIZE) {
        char *responseBuffer = allocate(responseSize);
        respUntilEmptyLineStr(resp, responseBuffer, responseSize);
        int result = sendAll(client, responseBuffer, responseSize);
        if (result == -1) {
            deallocate(responseBuffer);
            return;
        }
        if (resp->contentLength > 0) {
            result = sendAll(client, resp->content, resp->contentLength);
        }
        deallocate(responseBuffer);
    } else {
        int result = sendAll(client, stackBuffer, responseSize);
        if (result == -1) {
            return;
        }
        if (resp->contentLength > 0) {
            result = sendAll(client, resp->content, resp->contentLength);
            if (result == -1) {
                return;
            }
        }
    }
}

int sendAll(TcpSocket *client, void *buffer, int size) {
    int sent = 0;
    while (sent < size) {
        int packetSize = send(client->fd, buffer + sent, MIN(size - sent, 1 << 20), 0);
        if (packetSize == -1) {
            return -1;
        }
        sent += packetSize;
    }
    return 0;
}

void addEndpoint(char *path, HttpReqHandler handler) {
    if (router.capacity == -1) {
        router = emptyRouter();
    }
    HttpEndpoint endpoint = newEndpoint(path, handler);
    routerAddEndpoint(&router, endpoint);
}

void setNotFoundCallback(HttpReqHandler handler) {
    router.notFoundCallback = handler;
}
