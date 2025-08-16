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
        .errorType = 0,
        .error = 0,
    };

    assert(strnlen(port, 6) < 6);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        sock.errorType = EGAI;
        sock.error = status;
        return sock;
    }

    res->ai_flags = 0;
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_flags)) == -1)
    {
        freeaddrinfo(res);
        sock.errorType = ESOCK;
        sock.error = errno;
        return sock;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.errorType = ESOCK;
        sock.error = errno;
        return sock;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &(int){1}, sizeof(int)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.errorType = ESOCK;
        sock.error = errno;
        return sock;
    }

    if ((status = bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
    {
        freeaddrinfo(res);
        close(sockfd);
        sock.errorType = ESOCK;
        sock.error = errno;
        return sock;
    }

    freeaddrinfo(res);

    if ((status = listen(sockfd, 20)) == -1)
    {
        close(sockfd);
        sock.errorType = ESOCK;
        sock.error = errno;
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
        .error = 0,
    };
    int clientfd, sockfd = sock.fd;

    if ((clientfd = accept(sockfd, NULL, NULL)) == -1)
    {
        conn.error = errno;
        return conn;
    }

    conn.fd = clientfd;

    if (getClientIp(clientfd, conn.ip) == -1)
    {
        conn.error = errno;
        return conn;
    }

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
        .error = 0,
    };

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host, port, &hints, &res)) != 0)
    {
        conn.errorType = EGAI;
        conn.error = status;
        return conn;
    }

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        conn.errorType = ESOCK;
        conn.error = errno;
        return conn;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        conn.errorType = ESOCK;
        conn.error = errno;
        return conn;
    }

    conn.fd = sockfd;
    return conn;
}

void closeSocket(TcpSocket sock)
{
    close(sock.fd);
}

static char noErrorString[] = "No error";
static char unreachableErrorString[] = "Unreachable error";

const char *getSocketErrorString(TcpSocket sock)
{
    switch (sock.errorType)
    {
    case NO_ERROR:
        return noErrorString;
    case ESOCK:
        return strerror(sock.error);
    case EGAI:
        return gai_strerror(sock.error);
    default:
        return unreachableErrorString;
    }
}

int hasError(TcpSocket sock)
{
    return sock.errorType > 0;
}

void clearSocketError(TcpSocket *sock)
{
    sock->error = 0;
    sock->errorType = 0;
}

void receive(TcpSocket *sock, void *buffer, size_t size)
{
    if (recv(sock->fd, buffer, size, 0) == -1)
    {
        sock->errorType = ESOCK;
        sock->error = errno;
    }
}

void transmit(TcpSocket *sock, void *buffer, size_t size)
{
    if (send(sock->fd, buffer, size, 0) == -1)
    {
        sock->errorType = ESOCK;
        sock->error = errno;
    }
}

int getClientIp(int fd, char *str)
{
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof addr;
    int status = getpeername(fd, (struct sockaddr *)&addr, &addr_size);
    if (status == -1)
    {
        return -1;
    }
    inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);
    return 0;
}
