/**
 * @file rio.h
 * @brief Declarations of robust I/O.
 * @author qyl
 * @version 0.2
 * @date 2016-02-07
 */
#pragma once

#include "socket.h"

/** @struct _rio_t
 *  @brief This structure is the basic structure of robust I/O.
 *  @var _rio_t::rio_buf
 *  Member 'rio_buf' contains internal buffer.
 *  @var _rio_t::rio_bufptr
 *  Member 'rio_bufptr' contains next unread byte in internal buffer.
 *  @var _rio_t::rio_cnt
 *  Member 'rio_cnt' contains unread bytes in internal buffer.
 *  @var _rio_t::rio_fd
 *  Member 'rio_fd' contains descriptor for this internal buffer.
 */
typedef struct _rio_t {
    char rio_buf[MAXLINE];
    char *rio_bufptr;
    ssize_t rio_cnt;
    int rio_fd;
} rio_t;

/**
 * @brief Robustly read n bytes(unbuffered).
 *
 * @param fd The first parameter of read.
 * @param userbuf The buffer for reading.
 * @param n The number of bytes to be read.
 *
 * @return Number of read bytes.
 */
size_t readn(int fd, void *userbuf, size_t n);
/**
 * @brief Robustly write n bytes(unbuffered).
 *
 * @param fd The first parameter of write.
 * @param usrbuf The buffer for writing.
 * @param n The number of bytes to be written.
 *
 * @return Number of written bytes.
 */
ssize_t writen(int fd, const void *usrbuf, size_t n);

/**
 * @brief Associate a descriptor with a read buffer and reset buffer.
 *
 * @param rp The rio_t to be initialized.
 * @param fd The file descriptor to be associated.
 */
static inline void rio_init(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/**
 * @brief Read a line until \n.
 *
 * @param rp The rio_t to be read.
 * @param usrbuf The buffer for reading.
 * @param maxlen The max length to read.
 *
 * @return Number of read bytes.
 */
ssize_t readline(rio_t *rp, void *usrbuf, size_t maxlen);

