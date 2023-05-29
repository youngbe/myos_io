#include "threads.h"
#include "sched-internal.h"

#include <misc.h>

__attribute__((noinline)) int
mtx_trylock(struct Mutex *const mutex)
{
    struct Thread *const current_thread = thrd_current_inline();
    __asm__ (""::"r"(&current_thread->temp0):);
    al_node_init((al_node_t *)&current_thread->temp0);
    const bool is_sti = check_sti();
    if (is_sti)
        asm ("cli");
    const al_node_t *const current_node = al_head(&mutex->threads);
    if (current_node == NULL) {
        const int ret = al_append_empty_inline(&mutex->threads, (al_node_t *)&current_thread->temp0, false);
        if (is_sti)
            asm ("sti");
        if (ret == 0)
            return thrd_success;
        return thrd_busy;
    }
    if (is_sti)
        asm ("sti");
    //const struct Thread *const current_owner = list_entry((void *)current_node, struct Thread, temp0);
    if (&current_thread->temp0 == (void *)current_node) {
        if (mutex->count == 0 || mutex->count == SIZE_MAX)
            return thrd_error;
        ++mutex->count;
        return thrd_success;
    }
    return thrd_busy;
}
