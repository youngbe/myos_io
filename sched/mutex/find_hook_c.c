#include "sched-internal.h"
#include "mcs_spin.h"

#include <utlist.h>

#include <stdnoreturn.h>
#include <stdlib.h>
#include <stdatomic.h>

extern struct Core_Data kernel_gs_base;
// 这个函数用于已经切换至空线程 （栈换过去了， Core_Data的running_thread和current_proc也设置好了，进程的线程数量也加一了，即这个时候可以sti正常跑，但是咱们还是看看有没有新线程，然后再jmp empty_loop的情况）
noreturn void find_hook(void *, void *, struct Proc *const current_proc)
{
    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    __asm__ volatile("cli":::);

    atomic_fetch_sub_explicit(&idle_cores_num, 1, memory_order_relaxed);
    {
        ssize_t threads_num = atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed);
        while (true) {
            if (threads_num == 0) {
                atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
                __asm__ (
                        "sti\n\t"
                        "addq  $16, %%rsp\n\t"
                        "jmp   empty_loop"
                        :
                        :
                        :);
                __builtin_unreachable();
            }
            if (atomic_compare_exchange_strong_explicit(&schedulable_threads_num, &threads_num, threads_num - 1, memory_order_relaxed, memory_order_relaxed))
                break;
        }
    }

    spin_lock(&schedulable_threads_lock, &spin_mutex_member);
    struct Thread *fake_schedulable_threads = schedulable_threads;
    if (fake_schedulable_threads == NULL)
        __builtin_unreachable();
    struct Thread *const new_thread = fake_schedulable_threads;
    CDL_DELETE(fake_schedulable_threads, new_thread);
    schedulable_threads = fake_schedulable_threads;
    spin_unlock(&schedulable_threads_lock, &spin_mutex_member);
    {
        uint64_t *current_cr3;
        __asm__ (
                "movq   %%cr3, %0"
                :"=r"(current_cr3)
                :
                :);
        if (current_cr3 != new_thread->cr3) {
            __asm__ (
                    "movq   %0, %%cr3"
                    :
                    :"r"(new_thread->cr3)
                    :);
        }
    }
    // 必须先切换cr3再减，因为减完cr3可能被其他线程清除
    // 这里可能减为0,因为current_thread可能已经结束了
    if (atomic_fetch_sub_explicit(&current_proc->threads_num, 1, memory_order_relaxed) == 1) {
        // 清理尸体 current_proc
        // 待补充
        abort();
    }
    kernel_gs_base.running_thread = new_thread;
    kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];

    // 切换至新的上下文
    __asm__ (
            "movq   %0, %%rsp\n\t"
            "jmpq   *%1"
            :
            :"m"(new_thread->rsp), "m"(new_thread->return_hook)
            :);
    __builtin_unreachable();
}
