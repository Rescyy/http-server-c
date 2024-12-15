#ifndef CONNECTION_H
#define CONNECTION_H

#include <sys/types.h>

#define NO_ERROR   0
#define ESOCK      1
#define EGAI       2

typedef char port_t[6];

typedef struct TcpSocket {
    int fd;
    short error_type;
    short error;
    char ip[16];
} TcpSocket;

TcpSocket socketListen(port_t port);
TcpSocket acceptConnection(TcpSocket socket);
TcpSocket socketConnect(char *host, port_t port);
void closeSocket(TcpSocket sock);
const char *getSocketErrorString(TcpSocket sock);
void clearSocketError(TcpSocket *sock);
void receive(TcpSocket *sock, void *buffer, size_t size);
void transmit(TcpSocket *sock, void *buffer, size_t size);
int hasError(TcpSocket sock);
int getClientIp(int fd, char *ip);

#endif