/**
 * @file response.h
 * @brief Declarations related to responses.
 * @author qyl
 * @version 0.2
 * @date 2016-02-20
 */
#pragma once

#include "socket.h"
#include <stddef.h>

typedef enum {
    HTTP_OK = 200,
    HTTP_BAD_REQUEST = 400,
    HTTP_NOT_IMPLEMENTED = 500,
} status_e;

typedef enum {
    TEXT_HTML,
} content_type_e;

/** @struct _status_t
 *  @brief Structure represents status code in responses.
 *  @var _status_t::msg
 *  Member 'msg' contains corresponding message for the status code.
 *  @var _status_t::code
 *  Member 'code' contains status code.
 */
typedef struct _status_t {
    char *msg;
    status_e code;
} status_t;

extern const status_t STATUS_OK;
extern const status_t STATUS_BAD_REQUEST;
extern const status_t STATUS_NOT_IMPLEMENTED;

#define RESPONSE_HEADER_LEN 256
extern const char response_header[RESPONSE_HEADER_LEN];

#define ERR_BODY_LEN 512
extern const char err_body[ERR_BODY_LEN];

/**
 * @brief Send response to the client.
 *
 * @param fd The client's socket.
 * @param status The response's status code and corresponding message.
 * @param header_detail More options in header.
 * @param body The response body.
 * @param len Length of response body.
 */
void send_response(int fd, const status_t *status, const char *header_deatil,
            const char *body, size_t len);

