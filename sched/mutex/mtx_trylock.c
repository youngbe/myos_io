#include <threads.h>

#include "mtx_lock.h"

#include "sched-internal.h"
#include "thrd_current.h"

int mtx_trylock(mtx_t*const mtx)
{
    return mtx_trylockx(mtx, thrd_currentx());
}
