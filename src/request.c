#define _XOPEN_SOURCE 700

#include "request.h"
#include "rio.h"
#include "response.h"
#include "signals.h"
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <setjmp.h>
#include <strings.h>

static request_t *request = NULL;

#define TYPE_NUM 20

static const char *postfix[TYPE_NUM][3] = {
    {".html", ".htm",}, {".doc",}, {".ppt",}, {".gif",},
    {".jpeg", ".jpg", ".jpe",}, {".txt",},
};

static void init_req(void)
{
    request = (request_t *) calloc(1, sizeof(request_t));
}

static void free_req(void)
{
    free(request);
    request = NULL;
}

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

/* #ifdef debug */
/* static void print_request(void) */
/* { */
/*     printf("%u %s HTTP/%u.%u\n", request->method, request->request_url, */
/*             request->http_major, request->http_minor); */
/*     for (size_t j = 0; j < request->num_headers; ++j) { */
/*         printf("%s: %s\n", request->headers[j][0], request->headers[j][1]); */
/*     } */
/*     printf("%s\n", request->body); */

/*     printf("keep-alive: %u\n", request->should_keep_alive); */
/*     printf("body-size: %ld\n", request->body_size); */
/*     for (size_t j = 0; j < request->num_chunks; ++j) { */
/*         printf("chunk %zd size: %zd\n", j, request->chunk_lengths[j]); */
/*     } */
/*     printf("path: %s\n", request->request_path); */
/*     printf("query: %s\n", request->query_string); */
/*     printf("fragment: %s\n", request->fragment); */
/*     fputs("\n", stdout); */
/* } */
/* #endif */

static int headers_complete_cb(http_parser *p)
{
    request->method = p->method;
    request->http_major = p->http_major;
    request->http_minor = p->http_minor;
    request->should_keep_alive = http_should_keep_alive(p);
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

static size_t read_request(rio_t *rp, char *buf)
{
    ssize_t nread = readline(rp, buf, MAXLINE);
    size_t read_sum = nread;

    while (strncmp(buf, "\r\n", 2) != 0 && nread != 0) {
        buf += nread;
        nread = readline(rp, buf, MAXLINE);
        read_sum += nread;
    }
    return read_sum;
}

static volatile sig_atomic_t canjump;
static sigjmp_buf jmpbuf;

static void sigalarm_handler(int signo)
{
    assert(signo >= 0);
    if (canjump == 0) {
        // unexpected signal, ignore
        return;
    }
    canjump = 0;
    siglongjmp(jmpbuf, 1);
}

static int parse_request(int fd)
{
    const size_t buflen = 5 * MAXLINE;
    char buf[buflen];
    rio_t rio;

    rio_init(&rio, fd);
    canjump = 1;
    alarm(120);
    size_t nread = readline(&rio, buf, MAXLINE);

    alarm(0);
    if (nread == 0 || sigsetjmp(jmpbuf, 1)) {
        // sigsetjmp: no requests from the client within 2 mins, close connection
        return -1;
    }
    nread += read_request(&rio, buf+nread);
    if (strncmp(buf, "POST", 4) == 0) {
        nread += read_request(&rio, buf+nread);
    }
    http_parser parser;

    http_parser_init(&parser, HTTP_REQUEST);
    http_parser_settings settings = {
        .on_url = request_url_cb,
        .on_header_field = header_field_cb,
        .on_header_value = header_value_cb,
        .on_headers_complete = headers_complete_cb,
        .on_body = body_cb,
        .on_chunk_header = chunk_header_cb,
    };
    return nread == http_parser_execute(&parser, &settings, buf, nread);
}

static content_type_e get_type(const char *filename)
{
    for (size_t i = 0; i < TYPE_NUM; ++i) {
        for (size_t j = 0; postfix[i][j] != NULL; ++j) {
            if (strstr(filename,postfix[i][j]) != NULL) {
                return i;
            }
        }
    }
    return UNKNOWN;
}

static void serve_static(int fd, const char *filename, const struct stat *sbuf)
{
    content_type_e type = get_type(filename);
    time_t t = sbuf->st_mtim.tv_sec;
    const size_t TIME_BUF_SIZE = 32;
    char time_buf[TIME_BUF_SIZE];

    format_time(time_buf, TIME_BUF_SIZE, &t);
    char buf[256] = {'\0'};
    const char *header_details =
            "Last-Modified: %s\r\n"
            "Connection: keep-alive\r\n"
            "Accept-Range: bytes\r\n";

    snprintf(buf, 256, header_details, time_buf);
    if (request->method == HTTP_GET) {
        send_response(fd, &STATUS_OK, buf, type, true, filename, sbuf->st_size);
    } else {
        send_response(fd, &STATUS_OK, buf, type, false, NULL, sbuf->st_size);
    }
}

static void do_dup(int oldfd, int newfd)
{
    if (oldfd != newfd) {
        if (dup2(oldfd, newfd) != newfd) {
            unix_error("dup2 error");
        }
        if (close(oldfd) < 0) {
            unix_error("close error");
        }
    }
}

static void serve_dynamic(int fd, const char *filename)
{
    char buf[MAXLINE] = {'\0'};
    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Server: qweb\r\n"
        "Date: %s\r\n"
        "Connection: keep-alive\r\n";
    time_t t;
    const size_t TIME_BUF_SIZE = 32;
    char time_buf[TIME_BUF_SIZE];

    time(&t);
    format_time(time_buf, TIME_BUF_SIZE, &t);
    snprintf(buf, MAXLINE, header, time_buf);
    if (writen(fd, buf, strnlen(buf,MAXLINE)) < 0) {
        return;
    }
    pid_t pid;

    if ((pid = fork()) < 0) {
        send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
        unix_error("fork error");
    } else if (pid == 0) {
        do_dup(fd, STDOUT_FILENO);
        char *args[1];

        args[0] = request->method == HTTP_POST ? request->body : request->query_string;
        if (execvp(filename, args) < 0) {
            send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
            unix_error("execvp error");
        }
    }
    if (wait(NULL) < 0) {
        send_error(fd, &STATUS_INTERNAL_SERVER_ERROR, "Connection: close\r\n");
        unix_error("wait error");
    }
}

static bool do_method(int fd, const char *verify_modified)
{
    char filename[MAX_ELEMENT_SIZE] = ".";
    struct stat sbuf;
    const char *url = request->request_path;
    const size_t url_len = strnlen(url, MAX_ELEMENT_SIZE);

    strncat(filename, url, url_len);
    if (url[url_len-1] == '/') {
        strncat(filename, "home.html", 9);
    }
    if (stat(filename, &sbuf) < 0) {
        send_error(fd, &STATUS_NOT_FOUND, "Connection: keep-alive\r\n");
        return true;
    }
    if (strstr(filename,"/../") != NULL) {
        // can't access parent directory
        send_error(fd, &STATUS_FORBIDDEN, "Connection: keep-alive\r\n");
        return true;
    }
    struct tm t;

    memset(&t, 0, sizeof(struct tm));
    // %Y in strptime and %G in strftime
    if (verify_modified != NULL
            && strptime(verify_modified, "%a, %d %h %Y %T %Z", &t) == NULL) {
        send_error(fd, &STATUS_BAD_REQUEST, "Connection: close\r\n");
        return false;
    }
    // not set by strptime(); tells mktime() to determine if DST is in effect
    t.tm_isdst = -1;
    bool modified = mktime(&t) < sbuf.st_mtime;
    bool is_static = strstr(url, "/cgi-bin/") == NULL;
    enum http_method method = request->method;

    if (is_static && S_ISREG(sbuf.st_mode) && (S_IRUSR & sbuf.st_mode)) {
        if (method == HTTP_POST) {
            send_error(fd, &STATUS_METHOD_NOT_ALLOWED, "Connection: keep-alive\r\n");
        } else if (modified) {
            serve_static(fd, filename, &sbuf);
        } else {
            send_response(fd, &STATUS_NOT_MODIFIED, "Connection: keep-alive\r\n",
                    get_type(filename), false, NULL, 0);
        }
    } else if (!is_static && S_ISREG(sbuf.st_mode) && (S_IXUSR & sbuf.st_mode)) {
        if (method == HTTP_HEAD) {
            send_error(fd, &STATUS_METHOD_NOT_ALLOWED, "Connection: keep-alive\r\n");
        } else if (modified) {
            serve_dynamic(fd, filename);
        } else {
            send_response(fd, &STATUS_NOT_MODIFIED, "Connection: keep-alive\r\n",
                    get_type(filename), false, NULL, 0);
        }
    } else {
        send_error(fd, &STATUS_FORBIDDEN, "Connection: keep-alive\r\n");
    }
    return true;
}

static bool handle_headers(int fd)
{
/* #ifdef debug */
/*     print_request(); */
/* #endif */
    size_t headers_num = request->num_headers;
    bool has_host = false;
    const char *verify_modified = NULL;

    for (size_t i = 0; i < headers_num; ++i) {
        const char *header = request->headers[i][0];

        if (!has_host && strncmp(header,"Host",4) == 0) {
            has_host = true;
        } else if (verify_modified == NULL
                && strncmp(header,"If-Modified-Since",17) == 0) {
            verify_modified = request->headers[i][1];
        }
    }
    if (!has_host) {
        send_error(fd, &STATUS_BAD_REQUEST, "Connection: close\r\n");
        return false;
    }
    bool keep_alive = request->should_keep_alive;

    switch (request->method) {
    case HTTP_HEAD:
    case HTTP_GET:
    case HTTP_POST:
        if (!do_method(fd, verify_modified)) {
            keep_alive = false;
        }
        break;
    default: {
        const char *msg = keep_alive
                ? "Connection: keep-alive\r\n" : "Connection: close\r\n";

        send_error(fd, &STATUS_NOT_IMPLEMENTED, msg);
        }
        break;
    }
    return keep_alive;
}

void handle_request(int fd)
{
    const size_t MAX_REQUEST_PER_CONNECTION = 5;
    bool close = false;

    mysignal(SIGALRM, sigalarm_handler);
    for (size_t i = 0; i < MAX_REQUEST_PER_CONNECTION; ++i) {
        init_req();
        int res = parse_request(fd);

        if (res == 0) {
            send_error(fd, &STATUS_BAD_REQUEST, "Connection: close\r\n");
            close = true;
        } else if (res > 0) {
            if (!handle_headers(fd)) {
                close = true;
            }
        } else {
            close = true;
        }
        // else the client has closed the connection
        free_req();
        if (close) {
            break;
        }
    }
    mysignal(SIGALRM, SIG_DFL);
    Close(fd);
}

