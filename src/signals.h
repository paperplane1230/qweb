#pragma once

#include "error.h"
#include <signal.h>

typedef void handler_t(int);

handler_t *mysignal(int signum, handler_t *handler);

