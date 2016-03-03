#pragma once

#include "socket.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    HTTP_OK = 200,
    HTTP_NOT_MODIFIED = 304,
    HTTP_BAD_REQUEST = 400,
    HTTP_FORIBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED = 501,
} status_e;

typedef enum {
    TEXT_HTML, APPLICATION_MSWORD, APPLICATION_POWERPOINT,
    IMGAGE_GIF, IMAGE_JPEG, TEXT_PLAIN, UNKNOWN,
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
extern const status_t STATUS_NOT_MODIFIED;
extern const status_t STATUS_BAD_REQUEST;
extern const status_t STATUS_FORBIDDEN;
extern const status_t STATUS_NOT_FOUND;
extern const status_t STATUS_METHOD_NOT_ALLOWED;
extern const status_t STATUS_INTERNAL_SERVER_ERROR;
extern const status_t STATUS_NOT_IMPLEMENTED;
extern const status_t STATUS_VERSION_NOT_SUPPORTED;

extern const char response_header[MAXLINE];

#define ERR_BODY_LEN 512
extern const char err_body[ERR_BODY_LEN];

#define TIME_FORMAT "%a, %d %h %G %T %Z"

/**
 * @brief Send response to the client.
 *
 * @param fd The client's socket.
 * @param status The response's status code and corresponding message.
 * @param header_detail More options in header.
 * @param type Content-type.
 * @param send_file Whether the body is content of a file.
 * @param body The response body.
 * @param len Length of response body.
 */
void send_response(int fd, const status_t *status, const char *header_deatil,
        content_type_e type, bool send_file, const char *body, size_t len);

/**
 * @brief Send error messages to the client.
 *
 * @param fd The client's socket.
 * @param status Corresponding status code.
 * @param header_msg Corresponding other headers.
 */
void send_error(int fd, const status_t *status, const char *header_msg);

/**
 * @brief Format time.
 *
 * @param buf Buffer storing string.
 * @param size Parameter of strftime.
 * @param t Time to be formatted.
 */
void format_time(char *buf, size_t size, const time_t *t);

