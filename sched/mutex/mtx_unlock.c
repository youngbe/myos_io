#include "threads.h"
#include "sched-internal.h"

#include <stdatomic.h>

__attribute__((noinline)) int
mtx_unlock(struct Mutex *const mutex)
{
    const al_node_t *const current_node = al_head(&mutex->threads);
    if (current_node == NULL)
        return thrd_error;
    const struct Thread *const current_thread = thrd_current_inline();
    if (&current_thread->temp0 != (void *)current_node)
        return thrd_error;
    if (mutex->count > 1) {
        --mutex->count;
        return thrd_success;
    }
    atomic_thread_fence(memory_order_release);
    struct RET_al_delete_front temp_ret = al_delete_front2(&mutex->threads);
    if (temp_ret.next == NULL)
        return thrd_success;
    struct Thread *const new_owner = list_entry((void *)temp_ret.next, struct Thread, temp0);
    atomic_thread_fence(memory_order_release);
    if (check_sti())
        cli_set_thread_schedulable(new_owner);
    else
        set_thread_schedulable(new_owner);
    return thrd_success;
}
