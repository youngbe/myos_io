#include <threads.h>

#include "mtx_lock.h"
#include "sched-internal.h"
#include "spin_lock.h"

#include <utlist.h>

#include <stdlib.h>

static int __attribute__((always_inline)) mtx_lockx(mtx_t*const mtx, const size_t core_id, struct Thread **const _running_threads, struct Thread *const current_thread)
{
    /*
    __asm__ volatile(
            "nop\n\tnop"
            :
            :
            :"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "r8", "r9", "r10", "r11", "r12", "r13", "r14",
            "cc","rsp","r15","st",
                "mm0","mm1","mm2","mm3","mm4","mm5","mm6","mm7",
                "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15",
                "ymm0","ymm1","ymm2","ymm3","ymm4","ymm5","ymm6","ymm7","ymm8","ymm9","ymm10","ymm11","ymm12","ymm13","ymm14","ymm15",
                "zmm0","zmm1","zmm2","zmm3","zmm4","zmm5","zmm6","zmm7","zmm8","zmm9","zmm10","zmm11","zmm12","zmm13","zmm14","zmm15"
                );
*/


    const uint64_t temp_rsp = (uintptr_t)&core_exclusive_threads[core_id]->stack[0x200000];
    uint64_t current_rsp;
    __asm__ volatile(
            "movq   %%rsp, %0\n\t"
            "movq   %1, %%rsp"
            :"+r"(current_rsp)
            :"r"(temp_rsp)
            :);
    const bool sti = cli_spin_lock(&mtx->spin_mtx);
    // 确保当前owner != NULL
    {
        thrd_t mtx_owner = NULL;
        if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thread, memory_order_acquire, memory_order_relaxed)) {
            assert(mtx->count <= 1);
            sti_spin_unlock(&mtx->spin_mtx, sti);
            // 恢复上下文
            __asm__ volatile(
                    "movq   %0, %%rsp"
                    :
                    :"r"(current_rsp)
                    :);
            return thrd_success;
        }
    }
    // 接下来在sti_spin_unlock之前，owner都不会变成NULL
    DL_APPEND(mtx->blocked_threads, current_thread);
    sti_spin_unlock(&mtx->spin_mtx, sti);


    cli_spin_lock2((uint64_t *)&schedulable_threads_lock, sti);
    struct Thread *const new_thread = schedulable_threads;
    if (new_thread == NULL) {
        sti_spin_unlock((uint64_t *)&schedulable_threads_lock, sti);
        // 切换至专属Thread
        __asm__ volatile("jmp   empty_thread":::);
    }
    DL_DELETE2(schedulable_threads, new_thread, schedulable_threads_prev, schedulable_threads_next);
    sti_spin_unlock((uint64_t *)&schedulable_threads_lock, sti);
    _running_threads[core_id] = new_thread;

    // 切换至新的上下文
    __asm__ volatile(
            "movq   %0, %%rsp\n\t"
            "jmpq   *%1"
            :
            :"m"(new_thread->rsp), "m"(new_thread->return_hook)
            :);


    // 线程下次被唤醒就从这里开始运行
label_wakeup:
    return thrd_success;
}

int __attribute__((noinline)) mtx_lock(mtx_t*const mtx)
{
    const size_t core_id = get_core_id();
    struct Thread **const _running_threads = running_threads;
    struct Thread *const current_thread = _running_threads[core_id];
    {
        const int ret = mtx_trylockx(mtx, current_thread);
        if (ret != thrd_busy )
            return ret;
    }
    return mtx_lockx(mtx, core_id, _running_threads, current_thread);
}
