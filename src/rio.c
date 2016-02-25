/**
 * @file rio.c
 * @brief Implementation of functions in the header file.
 * @author qyl
 * @version 0.2
 * @date 2016-02-08
 */
#include "rio.h"
#include "response.h"

size_t readn(int fd, void *userbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread = 0;
    char *bufp = userbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else {
                send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
                unix_error("readn error");
            }
        } else if (nread == 0) {
            // EOF
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}

ssize_t writen(int fd, const void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    const char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR) {
                nwritten = 0;
            } else if (errno == EPIPE) {
                return -1;
            } else {
                send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
                unix_error("writen error");
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf)
{
    while (rp->rio_cnt <= 0) {
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0 && errno != EINTR) {
            return SYSCALL_ERR;
        } else if (rp->rio_cnt == 0) {
            // EOF
            return 0;
        }
        rp->rio_bufptr = rp->rio_buf;
    }
    --rp->rio_cnt;
    *usrbuf = *rp->rio_bufptr++;
    return 1;
}

ssize_t readline(rio_t *rp, void *usrbuf, size_t maxlen)
{
    ssize_t rc = 0;
    char c = '\0';
    char *bufp = usrbuf;
    ssize_t n = 0;

    for (n = 1; n < (ssize_t) maxlen; ++n) {
        if ((rc = rio_read(rp, &c)) == 1) {
            *bufp++ = c;
            if (c == '\n') {
                // newline is stored
                break;
            }
        } else if (rc == 0) {
            // EOF
            *bufp = '\0';
            return n - 1;
        } else {
            send_error(rp->rio_fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("readline error");
        }
    }
    *bufp = '\0';
    return n;
}

