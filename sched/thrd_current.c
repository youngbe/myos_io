#include "threads.h"
#include "sched-internal.h"

thrd_t thrd_current(void)
{
    return thrd_current_inline();
}
