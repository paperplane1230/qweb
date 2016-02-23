/**
 * @file main.c
 * @brief The 'main' part of qweb.
 * @author qyl
 * @version 0.1
 * @date 2016-01-30
 */
#include "request.h"
#include <stdio.h>
#include <stdbool.h>

static inline void usage(const char *name)
{
    fprintf(stderr, "Usage: %s <port>\n", name);
    exit(ARGS_NUM_ERR);
}

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
        usage(argv[0]);
    }
    int listenfd = tcp_listen(NULL, argv[1]);
    struct sockaddr_storage clientaddr;
    char host[MAXLINE] = {'\0'};
    char port[MAXLINE] = {'\0'};

    while (true) {
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        Getnameinfo((SA *) &clientaddr, clientlen, host, MAXLINE,
                port, MAXLINE, NI_NUMERICSERV);
        printf("Accepted connection from (%s, %s)\n", host, port);
        handle_request(connfd);
        Close(connfd);
    }
    return EXIT_SUCCESS;
}

