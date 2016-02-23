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

const status_t STATUS_OK = {.msg="OK", .code=HTTP_OK,};
const status_t STATUS_BAD_REQUEST = {.msg="Bad Request", .code=HTTP_BAD_REQUEST,};
const status_t STATUS_NOT_IMPLEMENTED = {.msg="Not Implemented", .code=HTTP_NOT_IMPLEMENTED,};

static const char *content_type[] = {
    "text/html",
};

const char response_header[RESPONSE_HEADER_LEN] =
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

void send_response(int fd, const status_t *status, const char *header_detail,
            const char *body, size_t len)
{
    char header[MAXLINE] = {'\0'};
    const size_t TIME_BUF_SIZE = 32;
    char time_buf[TIME_BUF_SIZE];
    time_t t;

    time(&t);
    struct tm *tmp = gmtime(&t);

    if (strftime(time_buf, TIME_BUF_SIZE, "%a, %d %h %G %T GMT", tmp) == 0) {
        unix_error("strftime error");
    }
    snprintf(header, RESPONSE_HEADER_LEN, response_header, status->code, status->msg,
            time_buf, content_type[TEXT_HTML], len, header_detail);
    writen(fd, header, strnlen(header,MAXLINE));
    writen(fd, body, len);
}

