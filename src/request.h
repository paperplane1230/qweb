#pragma once

#include "http_parser.h"
#include "socket.h"
#include <stdbool.h>

#define MAX_ELEMENT_SIZE 2048
#define MAX_HEADERS 13
#define MAX_CHUNKS 16

/** @struct _request_t
 *  @brief Structure storing information of requests.
 *  @var _request_t::headers
 *  Member 'headers' contains headers of the request in key/value form.
 *  @var _request_t::reqeust_url
 *  Member 'request_url' contains requested url.
 *  @var _request_t::request_path
 *  Member 'request_path' contains requested path.
 *  @var _request_t::query_string
 *  Member 'query_string' contains requested query.
 *  @var _request_t::fragment
 *  Member 'fragment' contains requested fragment.
 *  @var _request_t::body
 *  Member 'body' contains requested body.
 *  @var _request_t::chunk_lengths
 *  Member 'chunk_lengths' contains lengths of chunks in the request.
 *  @var _request_t::num_headers
 *  Member 'num_headers' contains numbers of headers.
 *  @var _request_t::num_chunks
 *  Member 'num_chunks' contains numbers of chunks.
 *  @var _request_t::body_size
 *  Member 'body_size' contains size of the body.
 *  @var _request_t::type
 *  Member 'type' contains type of the message.
 *  @var _request_t::method
 *  Member 'type' contains method of the request.
 *  @var _request_t::last_header_element
 *  Member 'last_header_element' is used for counting number of headers.
 *  @var _request_t::http_major
 *  Member 'http_major' contains the major version of the request.
 *  @var _request_t::http_minor
 *  Member 'http_minor' contains the minor version of the request.
 *  @var _request_t::should_keep_alive
 *  Member 'should_keep_alive' judges whether it's a persistent connection.
 */
typedef struct _request_t {
    char headers[MAX_HEADERS][2][MAX_ELEMENT_SIZE];
    char request_url[MAX_ELEMENT_SIZE];
    char request_path[MAX_ELEMENT_SIZE];
    char query_string[MAX_ELEMENT_SIZE];
    char fragment[MAX_ELEMENT_SIZE];
    char body[MAX_ELEMENT_SIZE];
    size_t chunk_lengths[MAX_CHUNKS];
    size_t num_headers;
    size_t num_chunks;
    size_t body_size;
    enum http_parser_type type;
    enum http_method method;
    enum { NONE, FIELD, VALUE, } last_header_element;
    unsigned short http_major;
    unsigned short http_minor;
    bool should_keep_alive;
} request_t;

/**
 * @brief Handle requests.
 *
 * @param fd Connected file descriptor for this request.
 */
void handle_request(int fd);

