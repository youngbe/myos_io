#include "sched-internal.h"

#include "mcs_spin.h"

#include <io.h>

#include <utlist.h>

#include <stdatomic.h>
#include <stdnoreturn.h>
#include <stdlib.h>
#include <stdio.h>


extern struct Core_Data kernel_gs_base;
noreturn void kkk(void)
{
    struct Thread *old_thread;
    __asm__ ("nop":"=c"(old_thread)::);
    if (old_thread != kernel_gs_base.running_thread)
        __builtin_unreachable();
    __asm__ (
            "movq  %%rsp, %0\n\t"
            "leaq  .Lreturn(%%rip), %1"
            :"=m"(old_thread->rsp), "=r"(old_thread->return_hook)
            :
            :);
    //current_thread->return_hook = &&label_return;

    struct Proc *old_proc;
    bool old_is_kernel;
    {
        const uintptr_t temp = (uintptr_t)old_thread->proc;
        old_is_kernel = temp & 1;
        old_proc = (struct Proc *)(uintptr_t)(temp & -2);
    }
    if (!old_is_kernel)
        atomic_fetch_add_explicit(&old_proc->threads_num, 1, memory_order_relaxed);

    struct Spin_Mutex_Member spin_mutex_member;
    struct Thread *new_thread;
    spin_mutex_member_init(&spin_mutex_member);

    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&schedulable_threads_lock, &spin_mutex_member);
    {
        new_thread = schedulable_threads;
        if (new_thread != NULL) {
            struct Thread *fake_schedulable_threads = new_thread;
            DL_APPEND(fake_schedulable_threads, old_thread);
            DL_DELETE(fake_schedulable_threads, new_thread);
            schedulable_threads = fake_schedulable_threads;
        }
    }
    spin_unlock(&schedulable_threads_lock, &spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);

    if (new_thread == NULL) {
        // 没有可切换线程，fast_exit
        if (!old_is_kernel)
            atomic_fetch_sub_explicit(&old_proc->threads_num, 1, memory_order_acquire);
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

    const uint64_t *const new_cr3 = new_thread->cr3;
    if (new_cr3 != NULL) {
        // new thread is not kernel thread
        const struct Proc *const new_proc = new_thread->proc;
        if (new_proc != old_proc)
            __asm__ volatile("movq  %1, %%cr3":"+m"(__not_exist_global_sym_for_asm_seq):"r"(new_cr3):);
        if (atomic_fetch_sub_explicit(&old_proc->threads_num, 1, memory_order_release) == 1) {
            __asm__ volatile ("jmp abort":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
        }
        kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    } else
        new_thread->proc = (struct Proc *)(uintptr_t)((uintptr_t)old_proc | 1);

    kernel_gs_base.running_thread = new_thread;
    __asm__ volatile (
            "movq   %1, %%rsp"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(new_thread->rsp)
            :);
    __asm__ volatile (
            "wrmsr"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
            :);
    atomic_fetch_add_explicit(&old_schedulable_threads_num, 1, memory_order_acq_rel);
    __asm__ volatile (
            "jmpq   *%1"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(new_thread->return_hook)
            :);
    __builtin_unreachable();
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
        ssize_t threads_num = atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed);
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
    struct Thread *new_thread;

    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&schedulable_threads_lock, p_spin_mutex_member);
    {
        struct Thread *fake_schedulable_threads = schedulable_threads;
        if (fake_schedulable_threads == NULL)
            __builtin_unreachable();
        new_thread = fake_schedulable_threads;
        DL_DELETE(fake_schedulable_threads, new_thread);
        schedulable_threads = fake_schedulable_threads;
    }
    spin_unlock(&schedulable_threads_lock, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);

    struct Proc *const old_proc = kernel_gs_base.proc;
    const uint64_t *const new_cr3 = new_thread->cr3;
    if (new_cr3 != NULL) {
        // new thread is not kernel thread
        const struct Proc *const new_proc = new_thread->proc;
        if (new_proc != old_proc)
            __asm__ volatile("movq  %1, %%cr3":"+m"(__not_exist_global_sym_for_asm_seq):"r"(new_cr3):);
        if (atomic_fetch_sub_explicit(&old_proc->threads_num, 1, memory_order_release) == 1) {
            __asm__ volatile ("jmp abort":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
            __asm__ volatile ("nop":::);
        }
        kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    } else
        new_thread->proc = (struct Proc *)(uintptr_t)((uintptr_t)old_proc | 1);

    kernel_gs_base.running_thread = new_thread;
    __asm__ volatile (
            "movq   %1, %%rsp"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(new_thread->rsp)
            :);
    __asm__ volatile (
            "wrmsr"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
            :);
    __asm__ volatile (
            "jmpq   *%1"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(new_thread->return_hook)
            :);
    __builtin_unreachable();
}
