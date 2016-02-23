/**
 * @file main.h
 * @brief Header file of main.c of qweb.
 * @author qyl
 * @version 0.1
 * @date 2016-01-30
 */
#pragma once

#include "error.h"
#include <sys/socket.h>
#include <unistd.h>

// simplifies calls to bind(), connect(), and accept()
typedef struct sockaddr SA;

#define MAXLINE 8192

static const int LISTENQ = 1024;

/**
 * @brief Create a tcp socket for listening 
 *
 * @param host The server host to listen.
 * @param serv The service(port) of the server.
 *
 * @return A connected socket descriptor if succeed else exit with errors.
 */
int tcp_listen(const char *host, const char *serv);

/**
 * @brief Wrapper function for setsockopt.
 *
 * @param sockfd The first parameter of setsockopt.
 * @param level The second parameter of setsockopt.
 * @param optname The third parameter of setsockopt.
 * @param optval The fourth parameter of setsockopt.
 * @param optlen The fifth parameter of setsockopt.
 */
static inline void Setsockopt(int sockfd, int level, int optname,
        const void *optval, socklen_t optlen)
{
    int res = 0;

    if ((res = setsockopt(sockfd, level, optname, optval, optlen)) < 0) {
        unix_error("Setsockopt error");
    }
}

/**
 * @brief Wrapper function for close.
 *
 * @param fd The parameter of close.
 */
static inline void Close(int fd)
{
    int res = 0;

    if ((res = close(fd)) < 0) {
        unix_error("Close error");
    }
}

/**
 * @brief Wrapper function for accept.
 *
 * @param sockfd The first parameter of accept.
 * @param addr The second parameter of accept.
 * @param addrlen The third parameter of accept.
 *
 * @return Returned value of accept if succeed else exit with error.
 */
static inline int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int res = 0;

    if ((res = accept(sockfd, addr, addrlen)) < 0) {
        unix_error("Accept error");
    }
    return res;
}

/**
 * @brief Wrapper function for getnameinfo.
 *
 * @param sa The first parameter of getnameinfo.
 * @param salen The second parameter of getnameinfo.
 * @param host The third parameter of getnameinfo.
 * @param hostlen The fourth parameter of getnameinfo.
 * @param serv The fifth parameter of getnameinfo.
 * @param servlen The sixth parameter of getnameinfo.
 * @param flags The seventh parameter of getnameinfo.
 */
static inline void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
        char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
    int res = 0;

    if ((res = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)) != 0) {
        gai_error(res, "Getnameinfo error");
    }
}

