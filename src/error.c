#include "error.h"
#include <stdbool.h>
#include <sys/syslog.h>

bool is_daemon = false;

void gai_error(int errcode, const char *msg)
{
    const char *err_msg = gai_strerror(errcode);

    if (is_daemon) {
        syslog(LOG_ERR, "%s: %s", msg, err_msg);
    } else {
        fprintf(stderr, "%s: %s\n", msg, err_msg);
    }
    exit(SYSCALL_ERR);
}

void unix_error(const char *msg)
{
    if (is_daemon) {
        syslog(LOG_ERR, "%s: %m", msg);
    } else {
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    }
    exit(SYSCALL_ERR);
}

