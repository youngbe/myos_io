#include <threads.h>

#include "sched-internal.h"

#include <stdlib.h>

extern const mi_heap_t _mi_heap_empty;
extern void thread_start(void);
int thrd_create(thrd_t *const thr, const thrd_start_t func, void *const arg)
{
    const thrd_t new_thread = malloc(sizeof(struct Thread));
    if (new_thread == NULL)
        return thrd_error;
    *(void **)&new_thread->al_node = NULL;
    new_thread->cr3 = NULL;
    new_thread->proc = 1;
    new_thread->is_killed = false;
    new_thread->__errno = 0;
    new_thread->_mi_heap_default = (mi_heap_t*)&_mi_heap_empty;
    
    new_thread->rsp = (void *)((uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])] - 16);
    ((void **)new_thread->rsp)[0] = (void *)func;
    ((void **)new_thread->rsp)[1] = arg;
    new_thread->return_hook = thread_start;

    if (check_sti())
        cli_set_thread_schedulable(new_thread);
    else
        set_thread_schedulable(new_thread);

    *thr = new_thread;
    return thrd_success;
}
