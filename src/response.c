/**
 * @file response.c
 * @brief Definitions related to responses.
 * @author qyl
 * @version 0.2
 * @date 2016-02-21
 */
#include "response.h"
#include "rio.h"
#include <time.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

const status_t STATUS_OK = {.msg="OK", .code=HTTP_OK,};
const status_t STATUS_BAD_REQUEST = {.msg="Bad Request", .code=HTTP_BAD_REQUEST,};
const status_t STATUS_FORBIDDEN = {.msg="Forbidden", .code=HTTP_FORIBIDDEN,};
const status_t STATUS_NOT_FOUND = {.msg="Not Found", .code=HTTP_NOT_FOUND,};
const status_t STATUS_NOT_IMPLEMENTED = {.msg="Not Implemented", .code=HTTP_NOT_IMPLEMENTED,};
const status_t STATUS_INTERNAL_SERVER_ERROR
                = {.msg="Internal Server Error", .code=HTTP_INTERNAL_SERVER_ERROR,};

static const char *content_type[] = {
    "text/html", "text/plain", "unknown",
};

const char response_header[MAXLINE] =
    "HTTP/1.1 %u %s\r\n"
    "Server: qweb\r\n"
    "Date: %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %zd\r\n"
    "%s"
    "\r\n";

const char err_body[ERR_BODY_LEN] =
    "<html>\r\n"
    "<head><title>error</title></head>\r\n"
    "<body bgcolor=\"white\">\r\n"
    "<center><h1>%u %s</h1></center>\r\n"
    "<hr><center>qweb</center>\r\n"
    "</body>\r\n"
    "</html>\r\n";

void format_time(char *buf, size_t size, const time_t *t)
{
    struct tm *tmp = gmtime(t);

    if (strftime(buf, size, TIME_FORMAT, tmp) == 0) {
        unix_error("strftime error");
    }
}

void send_response(int fd, const status_t *status, const char *header_detail,
        content_type_e type, bool send_file, const char *body, size_t len)
{
    char header[MAXLINE] = {'\0'};
    const size_t TIME_BUF_SIZE = 32;
    char time_buf[TIME_BUF_SIZE];
    time_t t;

    time(&t);
    format_time(time_buf, TIME_BUF_SIZE, &t);
    snprintf(header, MAXLINE, response_header, status->code, status->msg,
            time_buf, content_type[type], len, header_detail);
    if (send_file) {
        const int optval = 1;

        if (setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval)) < 0) {
            send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("setsockopt error");
        }
    }
    if (writen(fd, header, strnlen(header,MAXLINE)) < 0) {
        // SIGPIPE caught
        return;
    }
    if (send_file) {
        int file = open(body, O_RDONLY);

        if (file < 0) {
            send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("open error");
        }
        if (sendfile(fd, file, 0, len) < 0) {
            send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("sendfile error");
        }
        const int optval = 0;

        if (setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval)) < 0) {
            send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("setsockopt error");
        }
        Close(file);
    } else if (body != NULL) {
        writen(fd, body, len);
    }
}

void send_error(int fd, const status_t *status, const char *header_msg)
{
    char body[MAXLINE] = {'\0'};

    snprintf(body, ERR_BODY_LEN, err_body, status->code, status->msg);
    send_response(fd, status, header_msg, TEXT_HTML, false, body, strnlen(body, MAXLINE));
}

