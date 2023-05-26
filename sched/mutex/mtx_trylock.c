#include "threads.h"
#include "sched-internal.h"

#include <misc.h>

#include <stdint.h>

int mtx_trylock(struct Mutex *const mutex)
{
    const struct Thread *const current_thread = thrd_current_inline();
    const struct Thread *const current_owner = al_head(&mutex->threads);
    if (current_owner == current_thread) {
        if (mutex->count == 0 || mutex->count == SIZE_MAX)
            return thrd_error;
        else {
            ++mutex->count;
            atomic_signal_fence(memory_order_acquire);
            return thrd_success;
        }
    } else if (current_owner == NULL) {
        uint64_t rflags;
        __asm__ volatile (
                "pushfq\n\t"
                "popq   %0"
                :"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        if (al_append_empty(&mutex->threads, &current_thread->node, rflags & 512) == 0) {
            atomic_signal_fence(memory_order_acquire);
            return thrd_success;
        }
    }
    return thrd_busy;
}
