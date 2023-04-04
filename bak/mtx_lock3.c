#include <threads.h>

#include "mtx_lock.h"
#include "sched-internal.h"

#include <utlist.h>

#include <stdlib.h>


int __attribute__((noinline)) mtx_lock(mtx_t*const mtx)
{
    struct Temp
    {
        struct Thread *running_thread;
        void *const empty_stack_rsp;
    } *const _core_res = &core_res[get_core_id()];
    struct Thread *const current_thread = _core_res->running_thread;
    {
        const int ret = mtx_trylockx(mtx, current_thread);
        if (ret != thrd_busy )
            return ret;
    }

    // 保存上下文
    bool is_sti;
    uint64_t context_rsp;
    __asm__ (
            "pushfq\n\t"
            "pushq  %%rbx\n\t"
            "pushq  %%r12\n\t"
            "pushq  %%r13\n\t"
            "pushq  %%r14\n\t"
            "pushq  %%r15\n\t"
            "pushq  %%rbp\n\t"
            "movq   %%rsp, %0"
            :"+r"(context_rsp), "=r"(is_sti)
            :
            ://"rbx", "rbp", "r12", "r13", "r14", "r15"
            );//"cc", "rbx", "rbp", "r12", "r13", "r14", "r15",
            //"st", "mm0",
            //"zmm0", "zmm16");
    void *const empty_stack_rsp = _core_res->empty_stack_rsp;
    __asm__ (
            "cli"
            :
            :
            :);
    current_thread->rsp = context_rsp;
    current_thread->return_hook = &&label_wakeup;
    {
        uint64_t unlocked = 0;
        while (!atomic_compare_exchange_weak_explicit(&mtx->spin_mtx, &unlocked, 1, memory_order_relaxed, memory_order_relaxed))
            unlocked = 0;
    }
    // 确保当前owner != NULL
    {
        thrd_t mtx_owner = NULL;
        if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thread, memory_order_relaxed, memory_order_relaxed)) {
            assert(mtx->count <= 1);
            atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
            if (is_sti) {
                __asm__ (
                        "sti"
                        :
                        :
                        :
                        );
            }
            // 恢复上下文
            __asm__ (
                    "addq   $10, %%rsp"
                    :
                    :
                    :);
            return thrd_success;
        }
    }
    CDL_APPEND(mtx->blocked_threads, current_thread);

    {
        uint64_t unlocked = 0;
        while (!atomic_compare_exchange_weak_explicit(&schedulable_threads_lock, &unlocked, 1, memory_order_relaxed, memory_order_relaxed))
            unlocked = 0;
    }
    struct Thread *fake_schedulable_threads = schedulable_threads;
    if (fake_schedulable_threads == NULL) {
        atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_relaxed);
        extern alignas(4096) const uint64_t kernel_pt0[512];
        __asm__ (
                "movq   %0, %%cr3"
                :
                :"r"(&kernel_pt0)
                :);
        atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
        __asm__ volatile(
                "movq   %0, %%rsp\n\t"
                "sti\n\t"
                "jmp   empty_loop"
                :
                :"r"(empty_stack_rsp)
                :);
    } else {
        struct Thread *const new_thread = fake_schedulable_threads;
        CDL_DELETE2(fake_schedulable_threads, new_thread, schedulable_threads_prev, schedulable_threads_next);
        schedulable_threads = fake_schedulable_threads;
        atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_relaxed);
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
        atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
        _core_res->running_thread = new_thread;

        // 切换至新的上下文
        __asm__ volatile(
                "movq   %0, %%rsp\n\t"
                "jmpq   *%1"
                :
                :"m"(new_thread->rsp), "m"(new_thread->return_hook)
                :);
    }


    // 线程下次被唤醒就从这里开始运行
label_wakeup:
    return thrd_success;
}
