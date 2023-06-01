#include "threads.h"
#include "sched-internal.h"

#include <stdatomic.h>

__attribute__((noinline)) int
mtx_unlock(struct Mutex *const mutex)
{
    const struct Thread *const current_thread = thrd_current_inline();
    if (current_thread != *(void **)&mutex->owner)
        return thrd_error;
    if (mutex->count > 1) {
        --mutex->count;
        return thrd_success;
    }

    void *current_waiters = *(void *volatile *)&mutex->waiters;
    void *current_end = NULL;
    if (current_waiters == NULL) {
        current_end = *(void *volatile *)&mutex->wait_end;
        if (current_end == NULL)
            __builtin_unreachable();
        if (current_end == &mutex->waiters) {
            atomic_store_explicit(&mutex->owner, NULL, memory_order_relaxed);
            if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_end, NULL, memory_order_relaxed, memory_order_relaxed))
                return thrd_success;
        }
        while ((current_waiters = *(void *volatile *)&mutex->waiters) == NULL)
            __asm__ volatile ("pause");
    }
    void *next;
    if (current_end == NULL) {
        next = *(void *volatile*)current_waiters;
        if (next == NULL) {
            current_end = *(void *volatile *)&mutex->wait_end;
            if (current_end == current_waiters) {
                atomic_store_explicit(&mutex->waiters, NULL, memory_order_relaxed);
                if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed))
                    goto label_next;
            }
            while ((next = *(void *volatile*)current_waiters) == NULL)
                asm ("pause");
        }
        *(void *volatile *)&mutex->waiters = next;
    } else {
        if (current_end == current_waiters) {
            atomic_store_explicit(&mutex->waiters, NULL, memory_order_relaxed);
            if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed))
                goto label_next;
        }
        while ((next = *(void *volatile*)current_waiters) == NULL)
            asm ("pause");
        *(void *volatile *)&mutex->waiters = next;
    }
label_next:;
    struct Thread *const new_owner = list_entry(current_waiters, struct Thread, temp0);
    atomic_store_explicit(&mutex->owner, new_owner, memory_order_relaxed);
    if (check_sti())
        cli_set_thread_schedulable(new_owner);
    else
        set_thread_schedulable(new_owner);
    return thrd_success;
}
