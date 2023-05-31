#include "threads.h"
#include "sched-internal.h"

#include <misc.h>

__attribute__((noinline)) int
mtx_trylock(struct Mutex *const mutex)
{
    __asm__ volatile (""::"r"(&mutex->waiters):);
    const struct Thread *const current_owner = *(void **)&mutex->owner;
    if (current_owner == NULL) {
        void *current_wait_end = *(void *volatile *)&mutex->wait_end;
        if (current_wait_end == NULL && atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_wait_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed)) {
            atomic_store_explicit(&mutex->owner, thrd_current_inline(), memory_order_relaxed);
            return thrd_success;
        }
    } else if (current_owner == thrd_current_inline()) {
        if (mutex->count == 0 || mutex->count == SIZE_MAX)
            return thrd_error;
        ++mutex->count;
        return thrd_success;
    }
    return thrd_busy;
}
