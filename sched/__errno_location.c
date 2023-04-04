#include <errno.h>

#include "sched-internal.h"

#include <threads.h>

extern int *__errno_location (void)
{
    return &thrd_current()->__errno;
}
