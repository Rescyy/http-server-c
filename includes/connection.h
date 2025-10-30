//
// Created by Crucerescu Vladislav on 07.03.2025.
//

#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/types.h>

#define NO_ERROR   0
#define ESOCK      1
#define EGAI       2

typedef char port_t[6];

typedef struct TcpSocket {
    int fd;
    int closed;
    char ip[16];
} TcpSocket;

typedef enum WriteEnum {
    WRITE_OK = 0,
    WRITE_TIMEOUT,
    WRITE_CLOSED,
    WRITE_POLL_ERROR,
    WRITE_SEND_ERROR,
    WRITE_OPEN_ERROR,
    WRITE_SENDFILE_ERROR,
} WriteEnum;

typedef struct WriteResult {
    WriteEnum result;
    size_t sent;
} WriteResult;

typedef enum ReadEnum {
    READ_OK = 0,
    READ_TIMEOUT,
    READ_CLOSED,
    READ_POLL_ERROR,
    READ_RECV_ERROR,
} ReadEnum;

typedef struct ReadResult {
    ReadEnum result;
    size_t received;
} ReadResult;

TcpSocket socketListen(const port_t port);
TcpSocket acceptConnection(TcpSocket socket);
TcpSocket socketConnect(const char *host, const port_t port);
void closeSocket(TcpSocket *sock);
ReadEnum canRead(int fd, int timeoutMs);
ReadResult receive(TcpSocket *sock, void *buffer, size_t size);
WriteEnum canWrite(int fd, int timeoutMs);
WriteResult transmit(TcpSocket *sock, const void *buffer, size_t size);
int getClientIp(int fd, char *ip);

#endif //CONNECTION_H
