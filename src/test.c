#include "connection.h"
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#define N_THREADS 20

void *make_request(void *arg)
{
    (void)arg;
    TcpSocket socket = socketConnect("127.0.0.1", "1234");
    char message[] = "Message";
    int status = send(socket.fd, message, sizeof(message) - 1, 0);
    if (status == -1) {
        printf("Send Error: %s\n", strerror(errno));
    }

    close(socket.fd);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[N_THREADS];

    for (int i = 0; i < N_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, make_request, NULL);
    }
    for (int i = 0; i < N_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
}