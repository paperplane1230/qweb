#include "signals.h"

handler_t *mysignal(int signum, handler_t *handler)
{
    struct sigaction action;
    struct sigaction old_action;

    action.sa_handler = handler;
    // block sigs of type being handled
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (signum == SIGALRM) {
#ifdef SA_INTERRUPT
        action.sa_flags |= SA_INTERRUPT;
#endif
    } else {
        // restart syscalls if possible
        action.sa_flags |= SA_RESTART;
    }
    if (sigaction(signum, &action, &old_action) < 0) {
        unix_error("sigaction error");
    }
    return old_action.sa_handler;
}


