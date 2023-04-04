#include <threads.h>

#include <assert.h>
#include <stdatomic.h>

int __attribute__((noinline)) mtx_trylock(mtx_t*const mtx)
{
    const thrd_t current_thrd = thrd_current();
    thrd_t mtx_owner = NULL;
    if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thrd, memory_order_acquire, memory_order_relaxed)) {
        assert(mtx->count <= 1);
        return thrd_success;
    }
    if (mtx_owner == current_thrd) {
        if (mtx->count == 0 || mtx->count == SIZE_MAX)
            return thrd_error;
        else {
            ++mtx->count;
            return thrd_success;
        }
    }
    else
        return thrd_busy;
}
