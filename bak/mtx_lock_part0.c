#include <threads.h>

#include "mtx_lock.h"
#include "thrd_current.h"

int mtx_lockx(mtx_t *, __typeof__((core_res)), thrd_t current_thread);

int __attribute__((noinline)) mtx_lock(mtx_t *const mtx)
{
    const __auto_type fake_core_res = core_res;
    const thrd_t current_thread = thrd_currentx(fake_core_res);
    {
        const int ret = mtx_trylockx(mtx, current_thread);
        if (ret != thrd_busy)
            return ret;
    }
    return mtx_lockx(mtx, fake_core_res, current_thread);
}
