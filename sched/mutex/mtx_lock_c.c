#include <threads.h>

#include "sched-internal.h"
#include "mtx_lock.h"
#include "mcs_spin.h"
#include "thrd_current.h"

#include <utlist.h>

#include <stdatomic.h>
#include <stdlib.h>

extern struct Core_Data kernel_gs_base;
int mtx_lock(mtx_t*const mtx)
{
    const thrd_t current_thread = thrd_currentx();
    {
        const int ret = mtx_trylockx(mtx, current_thread);
        if (ret != thrd_busy)
            return ret;
    }


    uint64_t rflags;
    void *context_rsp;
    struct Spin_Mutex_Member *p_spin_mutex_member;
    struct Proc *const current_proc = current_thread->proc;
    atomic_fetch_add_explicit(&current_proc->threads_num, 1, memory_order_relaxed);
    // 保存上下文
    {
        uint64_t r1;
        __asm__ (
                "pushq      %%rbp\n\t"
                "pushq      %%r15\n\t"
                "pushq      %%r14\n\t"
                "pushq      %%r13\n\t"
                "pushq      %%r12\n\t"
                "pushq      %%rbx\n\t"
                "subq       $16, %%rsp\n\t"
                "fstcw      8(%%rsp)\n\t"
                "stmxcsr    (%%rsp)\n\t"
                "rdfsbaseq  %[r1]\n\t"
                "pushq      %[r1]\n\t"
                "pushfq\n\t"
                "movq       (%%rsp), %[rflags]\n\t"
                "leaq       -8(%%rsp), %[context_rsp]\n\t"
                "subq       $24, %%rsp\n\t"
                "movq       %%rsp, %[pmutex]"
                :[rflags]"=r"(rflags), [r1]"=r"(r1), [context_rsp]"=r"(context_rsp), [pmutex]"=r"(p_spin_mutex_member)
                :
                :
                );
    }
    spin_mutex_member_init(p_spin_mutex_member);
    const bool is_sti = rflags & 512;
    if (is_sti)
        __asm__ ("cli":::);
    {
        uint64_t temp;
        __asm__ volatile (
                "swapgs\n\t"
                "rdgsbaseq  %0\n\t"
                "swapgs\n\t"
                "movq   %0, 16(%%rsp)"
                :"=r"(temp)
                :
                :);
    }
    current_thread->rsp = context_rsp;
    current_thread->return_hook = &&label_wakeup;
    kernel_gs_base.proc = current_proc;
    kernel_gs_base.running_thread = NULL;
    // 切换至空线程不需要切换rsp0


    spin_lock(&mtx->spin_mtx, p_spin_mutex_member);
    // 确保当前owner != NULL
    {
        thrd_t mtx_owner = NULL;
        if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thread, memory_order_relaxed, memory_order_relaxed)) {
            assert(mtx->count <= 1);
            spin_unlock(&mtx->spin_mtx, p_spin_mutex_member);
            kernel_gs_base.running_thread = current_thread;
            if (is_sti)
                __asm__ ("sti":::);
            // 这里不可能减为0,因为current_thread还在
            atomic_fetch_sub_explicit(&current_proc->threads_num, 1, memory_order_relaxed);
            __asm__ ("addq  $104, %%rsp":::);
            return thrd_success;
        }
    }
    CDL_APPEND(mtx->blocked_threads, current_thread);
    spin_unlock(&mtx->spin_mtx, p_spin_mutex_member);

    // 这期间不能用栈

    // 需要在开中断之前标记这个处理器空闲
    atomic_fetch_add_explicit(&idle_cores_num, 1, memory_order_relaxed);
    __asm__ goto (
            "rdgsbaseq %%rsp\n\t"
            "addq   %0, %%rsp\n\t"
            "sti\n\t"
            "jmp   find_hook"
            :
            :"i"(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack) - 16),
            "d"(current_proc)
            :
            :label_wakeup);

    __builtin_unreachable();
    // 线程下次被唤醒就从这里开始运行
label_wakeup:
    __asm__ (
            "popq       %%r12\n\t"
            "swapgs\n\t"
            "wrgsbaseq  %%r12\n\t"
            "swapgs\n\t"
            "popfq\n\t"
            "popq       %%r13\n\t"
            "wrfsbaseq  %%r13\n\t"
            "ldmxcsr    (%%rsp)\n\t"
            "fldcw      8(%%rsp)\n\t"
            "addq       $16, %%rsp\n\t"
            "popq       %%rbx\n\t"
            "popq       %%r12\n\t"
            "popq       %%r13\n\t"
            "popq       %%r14\n\t"
            "popq       %%r15\n\t"
            "popq       %%rbp"
            :
            :
            :);
    return thrd_success;
}
