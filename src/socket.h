#pragma once

#include "error.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

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
 * @brief Wrapper function for close.
 *
 * @param fd The parameter of close.
 */
void Close(int fd);

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
void Getnameinfo(const struct sockaddr *sa, socklen_t salen,
        char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);

