/**
 * @file request.c
 * @brief Definitions of functions related to requests.
 * @author qyl
 * @version 0.2
 * @date 2016-02-19
 */
#include "request.h"
#include "rio.h"
#include "response.h"
#include <assert.h>

static request_t *request = NULL;

static size_t strlncat(char *dst, size_t len, const char *src, size_t n)
{
    size_t slen = strnlen(src, n);
    size_t dlen = strnlen(dst, len);

    if (dlen < len) {
        size_t rlen = len - dlen;
        size_t ncpy = slen < rlen ? slen : (rlen - 1);

        memcpy(dst+dlen, src, ncpy);
        dst[dlen+ncpy] = '\0';
    }
    return slen + dlen;
}

static void print_request(void)
{
    printf("%u %s HTTP/%u.%u\n", request->method, request->request_url,
            request->http_major, request->http_minor);
    for (size_t j = 0; j < request->num_headers; ++j) {
        printf("%s: %s\n", request->headers[j][0], request->headers[j][1]);
    }
    printf("%s\n", request->body);

    printf("keep-alive: %u\n", request->should_keep_alive);
    printf("body-size: %ld\n", request->body_size);
    for (size_t j = 0; j < request->num_chunks; ++j) {
        printf("chunk %zd size: %zd\n", j, request->chunk_lengths[j]);
    }
    printf("path: %s\n", request->request_path);
    printf("query: %s\n", request->query_string);
    printf("fragment: %s\n", request->fragment);
    fputs("\n", stdout);
}

static int message_begin_cb(http_parser *p)
{
    assert(p!=NULL);
    // IMPORTANT: calloc set the memory area to be 0
    request = (request_t *) calloc(1, sizeof(request_t));
    return 0;
}

static int headers_complete_cb(http_parser *p)
{
    request->method = p->method;
    request->http_major = p->http_major;
    request->http_minor = p->http_minor;
    request->should_keep_alive = http_should_keep_alive(p);
    return 0;
}

static int message_complete_cb(http_parser *p)
{
    assert(p!=NULL);
    print_request();
    free(request);
    request = NULL;
    return 0;
}

static int chunk_header_cb(http_parser *p)
{
    size_t chunk_idx = request->num_chunks;

    ++request->num_chunks;
    assert(chunk_idx<MAX_CHUNKS);
    request->chunk_lengths[chunk_idx] = p->content_length;
    return 0;
}

static int header_field_cb(http_parser *p, const char *buf, size_t len)
{
    assert(p!=NULL);
    strlncat(request->headers[request->num_headers][0], MAX_ELEMENT_SIZE, buf, len);
    if (request->last_header_element != FIELD) {
        ++request->num_headers;
        assert(request->num_headers<MAX_HEADERS);
    }
    request->last_header_element = FIELD;
    return 0;
}

static int header_value_cb(http_parser *p, const char *buf, size_t len)
{
    assert(p!=NULL);
    strlncat(request->headers[request->num_headers-1][1], MAX_ELEMENT_SIZE, buf, len);
    request->last_header_element = VALUE;
    return 0;
}

static int request_url_cb(http_parser *p, const char *buf, size_t len)
{
    assert(p!=NULL);
    strlncat(request->request_url, MAX_ELEMENT_SIZE, buf, len);
    struct http_parser_url u;

    http_parser_url_init(&u);
    http_parser_parse_url(request->request_url, len, 0, &u);
    enum http_parser_url_fields url_info[] = {UF_PATH, UF_QUERY, UF_FRAGMENT,};
    char *url_ptr[] = {request->request_path, request->query_string, request->fragment,};
    const size_t URL_ELEMENT_NUM = 3;

    for (size_t i = 0; i < URL_ELEMENT_NUM; ++i) {
        enum http_parser_url_fields type = url_info[i];

        if (u.field_set & (1<<type)) {
            strlncat(url_ptr[i], MAX_ELEMENT_SIZE,
                    request->request_url+u.field_data[type].off,
                    u.field_data[type].len);
        }
    }
    return 0;
}

static int body_cb(http_parser *p, const char *buf, size_t len)
{
    assert(p!=NULL);

    strlncat(request->body, MAX_ELEMENT_SIZE, buf, len);
    request->body_size += len;
    return 0;
}

static size_t read_reqeust(rio_t *rp, char *buf)
{
    ssize_t nread = readline(rp, buf, MAXLINE);
    size_t read_sum = nread;

    while (strncmp(buf, "\r\n", 2) != 0) {
        buf += nread;
        nread = readline(rp, buf, MAXLINE);
        read_sum += nread;
    }
    return read_sum;
}

void handle_request(int fd)
{
    const size_t buflen = 5 * MAXLINE;
    char buf[buflen];
    rio_t rio;

    rio_init(&rio, fd);
    size_t nread = readline(&rio, buf, MAXLINE);

    nread += read_reqeust(&rio, buf+nread);
    if (strncmp(buf, "POST", 4) == 0) {
        read_reqeust(&rio, buf+nread);
    }
    http_parser parser;

    http_parser_init(&parser, HTTP_REQUEST);
    http_parser_settings settings = {
        .on_message_begin = message_begin_cb,
        .on_url = request_url_cb,
        .on_header_field = header_field_cb,
        .on_header_value = header_value_cb,
        .on_headers_complete = headers_complete_cb,
        .on_body = body_cb,
        .on_chunk_header = chunk_header_cb,
        .on_message_complete = message_complete_cb,
    };
    size_t parsed = http_parser_execute(&parser, &settings, buf, nread);
    char body[MAXLINE] = {'\0'};
    const status_t *status = parsed == nread
                        ? &STATUS_NOT_IMPLEMENTED : &STATUS_BAD_REQUEST;

    snprintf(body, ERR_BODY_LEN, err_body, status->code, status->msg);
    send_response(fd, status, "Connection: close\r\n", body, strnlen(body, MAXLINE));
}

