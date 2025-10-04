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
#include <fcntl.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <errno.h>

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
WriteResult sendResponse(HttpResp *resp, TcpSocket *client);
WriteResult sendContent(HttpResp *resp, TcpSocket *client);
WriteResult sendFile(HttpResp *resp, TcpSocket *client);
int handleError(int result, TcpSocket *client, HttpReq *request);
int handleRequest(SessionState *state, TcpStream *stream);

void startApp(char *port) {
    fflush(stdout);
    signal(SIGPIPE, SIG_IGN);

    if (router.capacity == -1) {
        router = emptyRouter();
    }
    pthread_t thread1;

    TcpSocket socket = socketListen(port);
    if (socket.closed) {
        fatal("Failed listening to socket");
        exit(1);
    }
    info(NULL, "Listening to port %s", port);

    int connectionIndex = 1;

    for (;;) {
        SessionState *state = newSessionState();
        state->connectionIndex = connectionIndex++;
        TcpSocket clientSocket = acceptConnection(socket);

        info("Connection accepted from client %s", clientSocket.ip);
        if (clientSocket.closed) {
            error("Client connection error: %s", strerror(errno));
            deallocate(state);
        } else {
            state->clientSocket = clientSocket;
            pthread_create(&thread1, NULL, handleConnectionThreadCall, state);
            pthread_detach(thread1);
        }
    }
}

void *handleConnectionThreadCall(void *arg) {
    start_alloc_tracking();
    SessionState *state = arg;
    setCurrentThreadSessionState(state);
    handleConnection(state);
    info("Closing connection");
    closeSocket(state->clientSocket);
    stop_alloc_tracking();
    pthread_exit(NULL);
}

#define BUFFER_SIZE 1024

void handleConnection(SessionState *appState) {
    TcpStream stream = newTcpStream(&appState->clientSocket);

    while (handleRequest(appState, &stream)) {
        appState->requestIndex++;
    }

    freeTcpStream(&stream);
}

int handleRequest(SessionState *state, TcpStream *stream) {
    HttpReq request = {
        .appState = state
    };
    HttpResp resp;
    int result = parseRequestStream(&request, stream);
    int action = handleError(result, &state->clientSocket, &request);
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

    int connectionKeepAlive = isConnectionKeepAlive(&request);

    resp = routeReq(router, request);

    logResponse(&resp, &request);
    WriteResult sendResult = sendResponse(&resp, &state->clientSocket);
    switch (sendResult.result) {
        case WRITE_OK:
            break;
        case WRITE_CLOSED:
            warning("Peer closed connection while sending");
            break;
        case WRITE_TIMEOUT:
            warning("Timeout while sending");
            break;
        default:
            error("Failed sending response");
    }

    freeResp(&resp);
    freeReq(&request);
    tcpStreamDrain(stream);

    return connectionKeepAlive;
}

/*
 * Returns 1 if should close connection.
 * Returns 2 if should respond with error.
 */
int handleError(int result, TcpSocket *client, HttpReq *request) {
    HttpResp resp;
    SessionState *state = request->appState;
    switch (result) {
        case TCP_STREAM_ERROR:
            error("TCP Socket Had An Error.");
            logErrorResponse(request, "TCP Stream error");
            return 1;

        case TCP_STREAM_CLOSED:
            info("TCP Socket Was Closed.");
            return 1;

        case TCP_STREAM_TIMEOUT:
            info("Timeout Waiting For Client. Closing Connection.");
            return 1;

        case ENTITY_TOO_LARGE_ERROR:
            error("Received Entity Too Large.");
            resp = newResp(PAYLOAD_TOO_LARGE);
            break;

        case UNKNOWN_METHOD:
            error("Received Unknown Method.");
            resp = newResp(METHOD_NOT_ALLOWED);
            break;

        case UNKNOWN_VERSION:
            error("Received Unknown Version.");
            resp = newResp(HTTP_VERSION_NOT_SUPPORTED);
            break;

        case BAD_REQUEST_ERROR:
            error("Received Bad Request.");
            resp = newResp(BAD_REQUEST);
            break;

        case URI_TOO_LARGE_ERROR:
            error("Received URI Too Large.");
            resp = newResp(URI_TOO_LONG);
            break;

        default:
            return 0;
    }
    logResponse(&resp, request);
    sendResponse(&resp, client);
    return 2;
}

WriteResult sendResponse(HttpResp *resp, TcpSocket *client) {
    char stackBuffer[BUFFER_SIZE];
    int responseSize = buildRespStringUntilContent(resp, stackBuffer, BUFFER_SIZE);
    WriteResult respResult, contentResult;
    if (responseSize > BUFFER_SIZE) {
        /* if stackBuffer not big enough */
        char *responseBuffer = allocate(responseSize);
        buildRespStringUntilContent(resp, responseBuffer, responseSize);
        respResult = transmit(client, responseBuffer, responseSize);
        if (respResult.result != WRITE_OK) {
            deallocate(responseBuffer);
            return respResult;
        }
        contentResult = sendContent(resp, client);
        deallocate(responseBuffer);
        if (contentResult.result != WRITE_OK) {
            contentResult.sent += respResult.sent;
            return contentResult;
        }
    } else {
        respResult = transmit(client, stackBuffer, responseSize);
        if (respResult.result != WRITE_OK) {
            return respResult;
        }
        contentResult = sendContent(resp, client);
        if (contentResult.result != WRITE_OK) {
            contentResult.sent += respResult.sent;
            return contentResult;
        }
    }
    return (WriteResult) {.result = WRITE_OK, .sent = contentResult.sent + respResult.sent};
}

WriteResult sendContent(HttpResp *resp, TcpSocket *client) {
    if (resp->contentLength == 0) {
        return (WriteResult) {.result = WRITE_OK, .sent = 0};
    }
    if (resp->isContentFile) {
        debug("Sending file content %s", resp->content);
        return sendFile(resp, client);
    }
    return transmit(client, resp->content, resp->contentLength);
}

WriteResult sendFile(HttpResp *resp, TcpSocket *client) {
    int fd = open(resp->content, O_RDONLY);
    if (fd < 0) {
        char errorBuffer[256];
        snprintf(errorBuffer, sizeof(errorBuffer), "Error opening file %s for sendFile\n", (char*) resp->content);
        perror(errorBuffer);
        return (WriteResult) {.result = WRITE_OPEN_ERROR, .sent = 0};
    }

    off_t offset = 0;
    off_t remaining = resp->contentLength;

    while (remaining > 0) {
        WriteEnum writable = canWrite(client->fd, 10 * 1000);
        if (writable != WRITE_OK) {
            close(fd);
            return (WriteResult) {.result = writable, .sent = offset};
        }
        off_t tempOffset = offset;
        ssize_t sent = sendfile(client->fd, fd, &offset, remaining);
        debug("sendfile(%d, %d, %ld, %zu) returned %zd and changed offset to %ld", client->fd, fd, tempOffset, remaining, sent, offset);
        if (sent <= 0) {
            perror("sendfile");
            close(fd);
            return (WriteResult) {.result = WRITE_SENDFILE_ERROR, .sent = offset};
        }
        remaining -= sent;
    }
    close(fd);
    return (WriteResult) {.result = WRITE_OK, .sent = resp->contentLength};
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
