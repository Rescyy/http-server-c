//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#include "connection.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <poll.h>
#include <pthread.h>

#include "alloc.h"
#include "logging.h"
#include "utils.h"

/*
    return TcpSocket;
    error = 0: SUCCESS;
    error = -1: addrinfo error;
    error > 0: errno specific;
*/
TcpSocket socketListen(port_t port)
{
    int status, sockfd;
    socklen_t addrSize = sizeof(struct sockaddr_storage);
    struct addrinfo hints, *res;
    TcpSocket sock = {
        .fd = 0,
        .closed = 0,
    };

    assert(strnlen(port, 6) < 6);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "socketConnect: getaddrinfo; %s\n", gai_strerror(status));
        sock.closed = 1;
        return sock;
    }

    res->ai_flags = 0;
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_flags)) == -1)
    {
        freeaddrinfo(res);
        sock.closed = 1;
        perror("socketListen: socket");
        return sock;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.closed = 1;
        perror("socketListen: setsockopt");
        return sock;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.closed = 1;
        perror("socketListen: setsockopt");
        return sock;
    }

    if ((status = bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.closed = 1;
        perror("socketListen: bind");
        return sock;
    }

    freeaddrinfo(res);

    if ((status = listen(sockfd, 20)) == -1)
    {
        close(sockfd);
        sock.closed = 1;
        perror("socketListen: listen");
        return sock;
    }

    sock.fd = sockfd;
    return sock;
}

/*
    return TcpSocket;
    error = 0: SUCCESS;
    error = errno: errno specific;
*/
TcpSocket acceptConnection(TcpSocket sock)
{
    TcpSocket conn = {
        .fd = 0,
        .closed = 0,
    };
    int clientfd, sockfd = sock.fd;

    if ((clientfd = accept(sockfd, NULL, NULL)) == -1)
    {
        conn.closed = 1;
        return conn;
    }

    conn.fd = clientfd;

    getClientIp(clientfd, conn.ip);

    return conn;
}

/*
    return TcpSocket;
    error = 0: SUCCESS;
    error = -1: addrinfo error;
    error > 0: check errno;
*/
TcpSocket socketConnect(char *host, port_t port)
{
    struct addrinfo hints, *res;
    int sockfd, status;
    TcpSocket conn = {
        .fd = 0,
        .closed = 0,
    };

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "socketConnect: getaddrinfo; %s\n", gai_strerror(status));
        conn.closed = 1;
        return conn;
    }

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        perror("socketConnect: socket");
        conn.closed = 1;
        return conn;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("socketConnect: connect");
        conn.closed = 1;
        return conn;
    }

    conn.fd = sockfd;
    return conn;
}

void closeSocket(TcpSocket *sock)
{
    close(sock->fd);
}

ReadEnum canRead(int fd, int timeoutMs) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeoutMs);
    debug("poll({.fd = %d, .events = POLLIN}, 1, %d) returned %d and set .revents to %04x", fd, timeoutMs, ret, pfd.revents);
    if (ret == -1) {
        perror("canRead: poll");
        return READ_POLL_ERROR;
    }
    if (ret == 0) {
        return READ_TIMEOUT;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        return READ_CLOSED;
    }
    return READ_OK;
}

pthread_mutex_t socketLogMutex = PTHREAD_MUTEX_INITIALIZER;

ReadResult receive(TcpSocket *sock, void *buffer, size_t size) {
    if (sock->closed) {
        return (ReadResult) {
            .result = READ_CLOSED,
            .received = 0,
        };
    }

    ReadEnum readable = canRead(sock->fd, 60 * 1000);

    if (readable != READ_OK) {
        sock->closed = 1;
        return (ReadResult) {
            .result = readable,
            .received = -1,
        };
    }

    ssize_t recvd = recv(sock->fd, buffer, size, 0);
    debug("recv(%d, %p, %zu, 0) returned %zd", sock->fd, buffer, size, recvd);

    if (recvd == 0) {
        sock->closed = 1;
        return (ReadResult) {
            .result = READ_CLOSED,
            .received = 0,
        };
    }

    if (recvd == -1) {
        perror("receive: recv");
        sock->closed = 1;
        return (ReadResult) {
            .result = READ_RECV_ERROR,
            .received = -1,
        };
    }

    DECLARE_CURRENT_TIME(time);
    pthread_mutex_lock(&socketLogMutex);
    FILE *socketFile = fopen("socketLog.txt", "ab");
    fprintf(socketFile, "\n%s Received %zd bytes\nbegin:\n", time, recvd);
    fwrite(buffer, 1, recvd, socketFile);
    fclose(socketFile);
    pthread_mutex_unlock(&socketLogMutex);

    return (ReadResult) {
        .result = READ_OK,
        .received = recvd,
    };
}

WriteEnum canWrite(int fd, int timeoutMs) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;

    int ret = poll(&pfd, 1, timeoutMs);
    debug("poll({.fd = %d, .events = POLLOUT}, 1, %d) returned %d and set .revents to %04x", fd, timeoutMs, ret, pfd.revents);
    if (ret == -1) {
        perror("canWrite: poll");
        return WRITE_POLL_ERROR;
    }
    if (ret == 0) {
        return WRITE_TIMEOUT;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        return WRITE_CLOSED;
    }
    return WRITE_OK;
}

WriteResult transmit(TcpSocket *sock, const void *buffer, size_t size) {
    if (sock->closed) {
        return (WriteResult) {
            .result = WRITE_CLOSED,
            .sent = 0,
        };
    }
    size_t totalSent = 0;

    while (totalSent < size) {
        // Wait until writable (10s timeout per chunk)
        WriteEnum writable = canWrite(sock->fd, 10 * 1000);
        if (writable != WRITE_OK) {
            sock->closed = 1;
            return (WriteResult) {
                .result = writable,
                .sent = totalSent, // return how much was actually sent
            };
        }

        // Send up to 1 MB at a time
        size_t packetSize = (size - totalSent) > (1 << 20) ? (1 << 20) : (size - totalSent);
        ssize_t sent = send(sock->fd, (char*)buffer + totalSent, packetSize, 0);
        debug("send(%d, %p, %zu, 0) return %zd", sock->fd, buffer, packetSize, sent);

        if (sent == -1) {
            perror("transmit: send");
            sock->closed = 1;
            return (WriteResult) {
                .result = WRITE_SEND_ERROR,
                .sent = totalSent,
            };
        }

        if (sent == 0) {
            sock->closed = 1;
            return (WriteResult) {
                .result = WRITE_CLOSED,
                .sent = totalSent,
            };
        }

        DECLARE_CURRENT_TIME(time);
        pthread_mutex_lock(&socketLogMutex);
        FILE *socketFile = fopen("socketLog.txt", "ab");
        fprintf(socketFile, "\n%s Sent %zd bytes\nbegin:\n", time, sent);
        fwrite(buffer + totalSent, 1, sent, socketFile);
        fclose(socketFile);
        pthread_mutex_unlock(&socketLogMutex);

        totalSent += sent;
    }

    return (WriteResult) {
        .result = WRITE_OK,
        .sent = totalSent,
    };
}


int getClientIp(int fd, char *str)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof addr;
    int status = getpeername(fd, (struct sockaddr *)&addr, &addr_size);
    if (status == -1)
    {
        perror("getClientIp: getpeername");
        return -1;
    }
    inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
    return 0;
}
