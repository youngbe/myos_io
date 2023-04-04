#include "sched-internal.h"

#include "mcs_spin.h"

#include <io.h>

#include <utlist.h>

#include <stdatomic.h>
#include <stdnoreturn.h>
#include <stdlib.h>
#include <stdio.h>


extern struct Core_Data kernel_gs_base;
noreturn void kkk()
{
    struct Thread *current_thread;
    __asm__ ("nop":"=c"(current_thread)::);
    if (current_thread != kernel_gs_base.running_thread)
        __builtin_unreachable();
    __asm__ (
            "movq  %%rsp, %0"
            //"leaq  .Lreturn(%%rip), %1\n\t"
            :"=m"(current_thread->rsp)//, "=m"(current_thread->return_hook)
            :
            :);
    current_thread->return_hook = &&label_return;
    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    spin_lock(&schedulable_threads_lock, &spin_mutex_member);
    struct Thread *fake_schedulable_threads = schedulable_threads;

    if (fake_schedulable_threads == NULL) {
        spin_unlock(&schedulable_threads_lock, &spin_mutex_member);
        // 没有可切换线程，fast_exit
        __asm__ (
                "popq   %%rsp\n\t"
                "addq   $104, %%rsp\n\t"
                "wrmsr\n\t"
                "lock   incq    %0\n\t"
                "jmp    .Lpop4_iretq"
                :"+m"(old_schedulable_threads_num)
                :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
                :);
        __builtin_unreachable();
    }
    struct Thread *const new_thread = fake_schedulable_threads;
    CDL_REPLACE_ELEM(fake_schedulable_threads, new_thread, current_thread);
    schedulable_threads = fake_schedulable_threads->next;
    {
        // 切换cr3
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

    spin_unlock(&schedulable_threads_lock, &spin_mutex_member);

    kernel_gs_base.running_thread = new_thread;
    kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    __asm__ (
            "movq  %1, %%rsp\n\t"
            "wrmsr\n\t"
            "lock incq   %0\n\t"
            "jmpq  *%2"
            :"+m"(old_schedulable_threads_num)
            :"m"(new_thread->rsp), "m"(new_thread->return_hook),
            // 让编译器使用%rdi寄存器，配合fast_exit （否则编译器会使用%rsi）
            "D"(new_thread),
            "c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
            :);
label_return:
    // 恢复上下文
    __asm__ volatile(
            "nop"
            :
            :
            :);
    __builtin_unreachable();
}

noreturn void kkk2()
{
    /*
    struct Thread *current_thread;
    __asm__ ("nop":"=d"(current_thread)::);
    if (current_thread != kernel_gs_base.running_thread)
        __builtin_unreachable();
    if (current_thread != NULL)
        __builtin_unreachable();
        */

    atomic_fetch_sub_explicit(&idle_cores_num, 1, memory_order_relaxed);
    {
        ssize_t threads_num = atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed);
        while (true) {
            if (threads_num == 0) {
                atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
                static_assert(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack) == 65536);
                // 不需要切换cr3
                __asm__ (
                        "rdgsbaseq %%rsp\n\t"
                        "addq   %[size], %%rsp\n\t"
                        "wrmsr\n\t"
                        "sti\n\t"
                        "jmp   empty_loop"
                        :
                        :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0), [size]"i"(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack))
                        :);
                __builtin_unreachable();
            }
            if (atomic_compare_exchange_strong_explicit(&schedulable_threads_num, &threads_num, threads_num - 1, memory_order_relaxed, memory_order_relaxed))
                break;
        }
    }

    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    spin_lock(&schedulable_threads_lock, &spin_mutex_member);
    struct Thread *fake_schedulable_threads = schedulable_threads;
    if (fake_schedulable_threads == NULL)
        __builtin_unreachable();
    struct Thread *const new_thread = fake_schedulable_threads;
    CDL_DELETE(fake_schedulable_threads, new_thread);
    schedulable_threads = fake_schedulable_threads;
    spin_unlock(&schedulable_threads_lock, &spin_mutex_member);
    {
        // 切换cr3
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
    if (atomic_fetch_sub_explicit(&kernel_gs_base.proc->threads_num, 1, memory_order_relaxed) == 1) {
        // 清理尸体 current_proc
        // 待补充
        abort();
    }

    kernel_gs_base.running_thread = new_thread;
    kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    __asm__ (
            "movq  %0, %%rsp\n\t"
            "wrmsr\n\t"
            "jmpq  *%1"
            :
            :"m"(new_thread->rsp), "m"(new_thread->return_hook)
            , "c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
            :);
    __builtin_unreachable();
}
