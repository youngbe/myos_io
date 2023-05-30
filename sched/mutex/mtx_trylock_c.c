#include "threads.h"
#include "sched-internal.h"

#include <misc.h>

__attribute__((noinline)) int
mtx_trylock(struct Mutex *const mutex)
{
    const struct Thread *const current_thread = thrd_current_inline();
    //if (atomic_load_explicit(&mutex->owner, memory_order_relaxed) == current_thread) {
    if (*(void **)&mutex->owner == current_thread) {
        if (mutex->count == 0 || mutex->count == SIZE_MAX)
            return thrd_error;
        ++mutex->count;
        return thrd_success;
    }
    //__asm__ volatile (""::"r"(&mutex->wait_end), "r"(&mutex->waiters):);
    __asm__ volatile (""::"r"(&mutex->waiters):);
    const bool is_sti = check_sti();
    if (is_sti)
        asm ("cli");
    void *current_wait_end = *(void *volatile *)&mutex->wait_end;
    //void *current_wait_end = atomic_load_explicit(&mutex->wait_end, memory_order_relaxed);
    if (current_wait_end == NULL) {
        const bool success = atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_wait_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed);
        if (is_sti)
            asm ("sti");
        if (!success)
            return thrd_busy;
        atomic_store_explicit(&mutex->owner, (struct Thread *)current_thread, memory_order_relaxed);
        return thrd_success;
    }
    if (is_sti)
        asm ("sti");
    return thrd_busy;
}
