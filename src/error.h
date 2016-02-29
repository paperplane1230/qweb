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
void gai_error(int errcode, const char *msg);

/**
 * @brief Unix-style error.
 *
 * @param msg The message to be shown.
 */
void unix_error(const char *msg);

