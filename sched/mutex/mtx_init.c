#include "threads.h"

int mtx_init(struct Mutex* const mutex, const int type)
{
    if (type == mtx_plain)
        mutex->count = 0;
    else if (type == (mtx_plain | mtx_recursive))
        mutex->count = 1;
    else
        return thrd_error;
    *(struct Thread **)&mutex->owner = NULL;
    *(void **)&mutex->waiters = NULL;
    *(void **)&mutex->wait_end = NULL;
    return thrd_success;
}
