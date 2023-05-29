#include "sched-internal.h"

#include <spinlock.h>

#include <stdnoreturn.h>
/*
#include <misc.h>

#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
*/

//extern noreturn void switch_to_interrupt(struct Thread *new_thread, struct Proc *old_proc);
extern struct Core_Data kernel_gs_base;
noreturn void kkk(void)
{
    struct Thread *current_thread;
    __asm__ ("nop":"=c"(current_thread)::);
    if (current_thread != kernel_gs_base.running_thread)
        __builtin_unreachable();
    __asm__ (
            "movq  %%rsp, %0\n\t"
            "leaq  .Lreturn(%%rip), %1"
            :"=m"(current_thread->rsp), "=&r"(current_thread->return_hook)
            :
            :);
    //current_thread->return_hook = &&label_return;
    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    al_node_init(&current_thread->al_node);

    struct Proc *current_proc;
    bool current_is_kernel;
    {
        const uintptr_t temp = current_thread->proc;
        current_is_kernel = temp & 1;
        current_proc = (struct Proc *)(temp & -2);
    }
    if (!current_is_kernel)
        atomic_fetch_add_explicit(&current_proc->threads_num, 1, memory_order_relaxed);


    al_append_inline(&schedulable_threads, &current_thread->al_node, false);

    struct RET_al_delete_front temp_ret;
    spin_lock_inline(&schedulable_threads_lock, &spin_mutex_member);
    temp_ret = al_delete_front(&schedulable_threads);
    spin_unlock_inline(&schedulable_threads_lock, &spin_mutex_member);

    if (temp_ret.head == NULL)
        __builtin_unreachable();
    struct Thread *const new_thread = list_entry(temp_ret.head, struct Thread, al_node);

    if (new_thread == current_thread) {
        // 没有切换线程，fast_exit
        if (!current_is_kernel)
            atomic_fetch_sub_explicit(&current_proc->threads_num, 1, memory_order_acquire);
        __asm__ volatile (
                "popq   %%rsp\n\t"
                "addq   $96, %%rsp\n\t"
                "wrmsr"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
                :"cc");
        atomic_fetch_add_explicit(&old_schedulable_threads_num, 1, memory_order_release);
        __asm__ volatile (
                "jmp    .Lpop5_iretq"
                :
                :
                :);
        __builtin_unreachable();
    }
    atomic_fetch_add_explicit(&old_schedulable_threads_num, 1, memory_order_acq_rel);
    __asm__ volatile (
            "jmp        switch_to_interrupt"
            :
            :"D"(new_thread), "S"(current_proc)
            :);
    __builtin_unreachable();
    //switch_to_interrupt(new_thread, old_proc);
}

noreturn void kkk2(void)
{
    /*
    struct Thread *current_thread;
    __asm__ ("nop":"=d"(current_thread)::);
    if (current_thread != kernel_gs_base.running_thread)
        __builtin_unreachable();
    if (current_thread != NULL)
        __builtin_unreachable();
        */

    atomic_fetch_sub_explicit(&idle_cores_num, 1, memory_order_acquire);
    {
        size_t threads_num = atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed);
        while (true) {
            if (threads_num == 0) {
                atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
                static_assert(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack) == 65536);
                // 不需要切换cr3
                __asm__ volatile (
                        "rdgsbase %%rsp\n\t"
                        "addq   %[size], %%rsp\n\t"
                        "wrmsr\n\t"
                        "sti\n\t"
                        "jmp   empty_loop"
                        :
                        :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0), [size]"i"(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack))
                        :"cc");
                __builtin_unreachable();
            }
            if (atomic_compare_exchange_strong_explicit(&schedulable_threads_num, &threads_num, threads_num - 1, memory_order_relaxed, memory_order_relaxed))
                break;
            __asm__ volatile ("pause":::);
        }
    }

    struct Spin_Mutex_Member *p_spin_mutex_member;
    __asm__ volatile ("movq     %%rsp, %0":"=r"(p_spin_mutex_member)::);
    spin_mutex_member_init(p_spin_mutex_member);

    struct RET_al_delete_front temp_ret;
    spin_lock_inline(&schedulable_threads_lock, p_spin_mutex_member);
    temp_ret = al_delete_front(&schedulable_threads);
    spin_unlock_inline(&schedulable_threads_lock, p_spin_mutex_member);

    if (temp_ret.head == NULL)
        __builtin_unreachable();
    struct Thread *const new_thread = list_entry(temp_ret.head, struct Thread, al_node);

    //switch_to_interrupt(new_thread, kernel_gs_base.proc);
    __asm__ volatile (
            "jmp        switch_to_interrupt"
            :
            :"D"(new_thread), "S"(kernel_gs_base.proc)
            :);
    __builtin_unreachable();
}
