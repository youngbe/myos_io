#include "sched-internal.h"

#include "mcs_spin.h"

#include <io.h>

#include <utlist.h>

#include <stdnoreturn.h>

extern struct Core_Data kernel_gs_base;
// stack_safe
// cli
// 需要事先保存好当前线程上下文 rsp, return_hook, 并保存到阻塞列表/可调度列表（killed线程除外）
// 不会修改idle_cores_num
noreturn void __attribute__((noinline)) switch_to(struct Thread *const new_thread, struct Proc *const old_proc)
{
    const uint64_t *const new_cr3 = new_thread->cr3;
    if (new_cr3 != NULL) {
        // new thread is not kernel thread
        const struct Proc *const new_proc = new_thread->proc;
        if (new_proc != old_proc)
            __asm__ volatile("movq  %1, %%cr3":"+m"(__not_exist_global_sym_for_asm_seq):"r"(new_cr3):);
        if (atomic_fetch_sub_explicit(&old_proc->threads_num, 1, memory_order_release) == 1) {
            // 清理尸体 current_proc
            // 待补充
            struct Spin_Mutex_Member *p_spin_mutex_member;
            __asm__ volatile (
                    "leaq   -16(%%rsp), %[p_spin_mutex_member]"
                    :[p_spin_mutex_member]"=r"(p_spin_mutex_member)
                    :
                    :);
            p_spin_mutex_member = (struct Spin_Mutex_Member *)((uintptr_t)p_spin_mutex_member & -16);
            __asm__ volatile ("jmp  abort":::);
        }
        kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    } else
        new_thread->proc = (struct Proc *)(uintptr_t)((uintptr_t)old_proc | 1);

    kernel_gs_base.running_thread = new_thread;

    // 切换至新的上下文
    __asm__ volatile (
            "movq   %0, %%rsp\n\t"
            "jmpq   *%1"
            :
            :"m"(new_thread->rsp), "m"(new_thread->return_hook)
            :);
    __builtin_unreachable();
}

noreturn void __attribute__((noinline)) switch_to_interrupt(struct Thread *const new_thread, struct Proc *const old_proc)
{
    const uint64_t *const new_cr3 = new_thread->cr3;
    if (new_cr3 != NULL) {
        // new thread is not kernel thread
        const struct Proc *const new_proc = new_thread->proc;
        if (new_proc != old_proc)
            __asm__ volatile("movq  %1, %%cr3":"+m"(__not_exist_global_sym_for_asm_seq):"r"(new_cr3):);
        if (atomic_fetch_sub_explicit(&old_proc->threads_num, 1, memory_order_release) == 1) {
            // 清理尸体 current_proc
            // 待补充
            struct Spin_Mutex_Member *p_spin_mutex_member;
            __asm__ volatile (
                    "leaq   -16(%%rsp), %[p_spin_mutex_member]"
                    :[p_spin_mutex_member]"=r"(p_spin_mutex_member)
                    :
                    :);
            p_spin_mutex_member = (struct Spin_Mutex_Member *)((uintptr_t)p_spin_mutex_member & -16);
            __asm__ volatile ("jmp  abort":::);
        }
        kernel_gs_base.tss.rsp0 = (uintptr_t)&new_thread->stack[sizeof(new_thread->stack) / sizeof(new_thread->stack[0])];
    } else
        new_thread->proc = (struct Proc *)(uintptr_t)((uintptr_t)old_proc | 1);

    kernel_gs_base.running_thread = new_thread;

    // 切换至新的上下文
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

// rsp == empty_stack_top - 16

// rsp == empty_stack_top - 16
// current_proc : 普通线程：当前进程
// 内核线程：对应虚拟进程
noreturn void switch_to_empty(struct Proc *const current_proc)
{
    atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
    kernel_gs_base.running_thread = NULL;
    kernel_gs_base.proc = current_proc;

    atomic_signal_fence(memory_order_release);
    __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acquire);

    struct Spin_Mutex_Member *p_spin_mutex_member;
    __asm__ volatile ("movq     %%rsp, %0":"=r"(p_spin_mutex_member), "+m"(__not_exist_global_sym_for_asm_seq)::);
    spin_mutex_member_init(p_spin_mutex_member);

    atomic_signal_fence(memory_order_release);
    __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acquire);

    atomic_fetch_sub_explicit(&idle_cores_num, 1, memory_order_acquire);
    {
        ssize_t threads_num = atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed);
        while (true) {
            if (threads_num == 0) {
                atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
                __asm__ volatile (
                        "sti\n\t"
                        "addq  $16, %%rsp\n\t"
                        "jmp   empty_loop"
                        :
                        :
                        :"cc");
                __builtin_unreachable();
            }
            if (atomic_compare_exchange_strong_explicit(&schedulable_threads_num, &threads_num, threads_num - 1, memory_order_relaxed, memory_order_relaxed))
                break;
            __asm__ volatile ("pause":::);
        }
    }

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

    __asm__ volatile ("jmp  switch_to"::"D"(new_thread), "S"(current_proc):);
    __builtin_unreachable();
}
