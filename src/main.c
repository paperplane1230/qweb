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
#include <signal.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s <port>\n", name);
    exit(ARGS_NUM_ERR);
}

typedef void handler_t(int);

static handler_t *mysignal(int signum, handler_t *handler)
{
    struct sigaction action;
    struct sigaction old_action;

    action.sa_handler = handler;
    // block sigs of type being handled
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
        action.sa_flags |= SA_INTERRUPT;
#endif
    } else {
        // restart syscalls if possible
        action.sa_flags |= SA_RESTART;
    }
    if (sigaction(signum, &action, &old_action) < 0) {
        unix_error("sigaction error");
    }
    return old_action.sa_handler;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argv[0]);
    }
    int listenfd = tcp_listen(NULL, argv[1]);
    struct sockaddr_storage clientaddr;
    char host[MAXLINE] = {'\0'};
    char port[MAXLINE] = {'\0'};

    mysignal(SIGPIPE, SIG_IGN);
    while (true) {
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        Getnameinfo((SA *) &clientaddr, clientlen, host, MAXLINE,
                port, MAXLINE, NI_NUMERICSERV|NI_NUMERICHOST);
        printf("Accepted connection from (%s, %s)\n", host, port);
        handle_request(connfd);
        Close(connfd);
    }
    return EXIT_SUCCESS;
}

