/**
 * @file socket.c
 * @brief The definition of functions in socket.h.
 * @author qyl
 * @version 0.1
 * @date 2016-02-05
 */
#include "socket.h"
#include <netdb.h>
#include <string.h>

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int res = 0;

    if ((res = accept(sockfd, addr, addrlen)) < 0) {
        unix_error("Accept error");
    }
    return res;
}

void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
        char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
    int res = 0;

    if ((res = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)) != 0) {
        gai_error(res, "Getnameinfo error");
    }
}

void Close(int fd)
{
    if (close(fd) < 0) {
        unix_error("Close error");
    }
}

int tcp_listen(const char *host, const char *serv)
{
    struct addrinfo hints;

    // get a list of potential server addresses
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG | AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;
    int n = 0;
    struct addrinfo *res = NULL;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
        gai_error(n, "tcp_listen error");
    }
    struct addrinfo *ressave = res;
    int listenfd = 0;

    do {
        if ((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
            continue;
        }
        const int optval = 1;

        // eliminate "Address already in use" error from bind
        if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            unix_error("setsockopt error");
        }
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) {
            // success
            break;
        }
        Close(listenfd);
    } while ((res = res->ai_next) != NULL);
    // error from final socket() or bind()
    if (res == NULL) {
        unix_error("tcp_listen error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        Close(listenfd);
        unix_error("listen error");
    }
    freeaddrinfo(ressave);
    ressave = NULL;
    return listenfd;
}

