/**
 * @file main.c
 * @brief The 'main' part of qweb.
 * @author qyl
 * @version 0.1
 * @date 2016-01-30
 */
#include "socket.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * @brief The main function of qweb.
 *
 * @param argc The number of arguments.
 * @param argv[] The list of arguments.
 *
 * @return The status code standing for the situation of execution.
 */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int listenfd = tcp_listen(NULL, argv[1]);
    struct sockaddr_storage clientaddr;
    char hostname[MAXLINE];
    char port[MAXLINE];

    while (true) {
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                port, MAXLINE, NI_NUMERICSERV);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        Close(connfd);
    }
    return 0;
}

