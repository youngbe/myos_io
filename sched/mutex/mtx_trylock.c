#include "threads.h"
#include "sched-internal.h"

#include <misc.h>

#include <stdint.h>

int mtx_trylock(struct Mutex *const mutex)
{
    struct Thread *const current_thread = thrd_current_inline();
    const struct Thread *const current_owner = ({
            void *temp = al_head(&mutex->threads);
            if (temp != NULL)
                temp = list_entry(temp, struct Thread, temp0);
            temp;
            });
    if (current_owner == current_thread) {
        if (mutex->count == 0 || mutex->count == SIZE_MAX)
            return thrd_error;
        ++mutex->count;
        atomic_signal_fence(memory_order_acquire);
        return thrd_success;
    }
    const uint64_t rflags = ({
            uint64_t temp;
            __asm__ volatile (
                    "pushfq\n\t"
                    "popq   %0"
                    :"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                    :
                    :);
            temp;
            });
    const bool is_sti = rflags & 512;
    if (current_owner == NULL) {
        if (al_append_empty(&mutex->threads, (_Atomic(void *) *)&current_thread->temp0, is_sti) == 0) {
            atomic_signal_fence(memory_order_acquire);
            return thrd_success;
        }
    }
    return thrd_busy;
}
