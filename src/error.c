/**
 * @file error.c
 * @brief Definitions related to errors.
 * @author qyl
 * @version 0.3
 * @date 2016-02-24
 */
#include "error.h"

void gai_error(int errcode, const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(errcode));
    exit(SYSCALL_ERR);
}

void unix_error(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(SYSCALL_ERR);
}

