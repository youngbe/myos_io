#include <threads.h>

#include "mcs_spin.h"

#include <stdatomic.h>

int mtx_init(mtx_t* const mtx, int type)
{
    mtx->owner = NULL;
    mtx->blocked_threads = NULL;
    spin_mutex_init(&mtx->spin_mtx);
    if (type == mtx_plain)
        mtx->count = 0;
    else if (type == (mtx_plain | mtx_recursive))
        mtx->count = 1;
    else
        return thrd_error;
    return thrd_success;
}
