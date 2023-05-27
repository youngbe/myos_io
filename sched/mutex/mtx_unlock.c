#include "threads.h"
#include "sched-internal.h"

#include <stdatomic.h>

int mtx_unlock(struct Mutex *const mutex)
{
    const struct Thread *const current_owner = ({
            void *temp = al_head(&mutex->threads);
            if (temp != NULL)
                temp = list_entry(temp, struct Thread, temp0);
            temp;
            });
    if (current_owner == NULL)
        return thrd_error;
    const struct Thread *const current_thread = thrd_current_inline();
    if (current_thread != current_owner)
        return thrd_error;
    if (mutex->count > 1) {
        --mtx->count;
label_return:
        atomic_thread_fence(memory_order_release);
        return thrd_success;
    }
    struct RET_al_delete_front temp_ret = al_delete_front2(&mutex->threads);
    if (temp_ret.next == NULL)
        goto label_return;
    const struct Thread *const new_owner = list_entry((void *)temp_ret.next, struct  Thread, temp0);
    atomic_thread_fence(memory_order_release);
    set_thread_schedulable(new_owner, get_interrupt_status());
    goto label_return;
}
