/**
 * @file signals.h
 * @brief Declarations related to signals.
 * @author qyl
 * @version 0.3
 * @date 2016-02-26
 */
#pragma once

#include "error.h"
#include <signal.h>

typedef void handler_t(int);

handler_t *mysignal(int signum, handler_t *handler);

