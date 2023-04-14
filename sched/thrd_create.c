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
    new_thread->cr3 = NULL;
    new_thread->proc = (struct Proc *)(uintptr_t)1;
    new_thread->is_killed = false;
    new_thread->__errno = 0;
    new_thread->_mi_heap_default = (mi_heap_t*)&_mi_heap_empty;
    
    new_thread->rsp = (void *)((uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])] - 16);
    ((void **)new_thread->rsp)[0] = (void *)func;
    ((void **)new_thread->rsp)[1] = arg;
    new_thread->return_hook = thread_start;

    {
        uint64_t rflags;
        __asm__ volatile(
                "pushfq\n\t"
                "popq   %0"
                :"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        struct Spin_Mutex_Member spin_mutex_member;
        spin_mutex_member_init(&spin_mutex_member);
        set_thread_schedulable(new_thread, rflags & 512, &spin_mutex_member);
    }

    *thr = new_thread;
    return thrd_success;
}
