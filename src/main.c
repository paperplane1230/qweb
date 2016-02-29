#include "request.h"
#include "signals.h"
#include <stdio.h>
#include <sys/syslog.h>

static void usage(const char *name)
{
    fprintf(stderr, "Usage: %s <port>\n", name);
    exit(ARGS_NUM_ERR);
}

extern bool is_daemon;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argv[0]);
    }
    if (daemon(1,0) < 0) {
        unix_error("daemon error");
    }
    is_daemon = true;
    int listenfd = tcp_listen(NULL, argv[1]);
    struct sockaddr_storage clientaddr;
    char host[MAXLINE] = {'\0'};
    char port[MAXLINE] = {'\0'};

    mysignal(SIGPIPE, SIG_IGN);
    while (true) {
        socklen_t clientlen = sizeof(clientaddr);
        int connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        Getnameinfo((SA *) &clientaddr, clientlen, host, MAXLINE,
                port, MAXLINE, NI_NUMERICSERV | NI_NUMERICHOST);
        syslog(LOG_INFO, "Connection from (%s, %s)", host, port);
        handle_request(connfd);
    }
    return EXIT_SUCCESS;
}

