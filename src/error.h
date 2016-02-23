/**
 * @file error.h
 * @brief Declaration of functions used to deal with errors.
 * @author qyl
 * @version 0.1
 * @date 2016-02-05
 */
#pragma once

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

typedef enum {
    SYSCALL_ERR = -1,
    ARGS_NUM_ERR = -2,
} error_e;

/**
 * @brief Getaddrinfo-style error.
 *
 * @param errcode The error code passed to gai_strerror.
 * @param msg The message to be shown.
 */
static inline void gai_error(int errcode, const char *msg) /* Getaddrinfo-style error */
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(errcode));
    exit(SYSCALL_ERR);
}

/**
 * @brief Unix-style error.
 *
 * @param msg The message to be shown.
 */
static inline void unix_error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(SYSCALL_ERR);
}

