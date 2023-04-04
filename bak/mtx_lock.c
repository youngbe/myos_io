#include <threads.h>

#include "mtx_lock.h"
#include "sched-internal.h"
#include "spin_lock.h"

#include <utlist.h>

#include <stdlib.h>

static int __attribute__((noinline)) mtx_lockx(mtx_t*const mtx, const size_t core_id, struct Thread **const _running_threads, struct Thread *const current_thread)
{
    __asm__ volatile(
            ""
            :
            :
            :"cc","rsp","r15","st",
                "mm0","mm1","mm2","mm3","mm4","mm5","mm6","mm7",
                "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15",
                "ymm0","ymm1","ymm2","ymm3","ymm4","ymm5","ymm6","ymm7","ymm8","ymm9","ymm10","ymm11","ymm12","ymm13","ymm14","ymm15",
                "zmm0","zmm1","zmm2","zmm3","zmm4","zmm5","zmm6","zmm7","zmm8","zmm9","zmm10","zmm11","zmm12","zmm13","zmm14","zmm15"
                );
    const bool sti = cli_spin_lock(&mtx->spin_mtx);
    // 确保当前owner != NULL
    {
        thrd_t mtx_owner = NULL;
        if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thread, memory_order_acquire, memory_order_relaxed)) {
            assert(mtx->count <= 1);
            sti_spin_unlock(&mtx->spin_mtx, sti);
            __asm__ volatile(
                    "popq   %%rsp\n\t"
                    "addq   $0x200, %%rsp\n\t"
                    :
                    :
                    :);
            // 恢复上下文
            return thrd_success;
        }
    }
    // 接下来在sti_spin_unlock之前，owner都不会变成NULL
    CDL_APPEND(mtx->blocked_threads, current_thread);
    // 该线程已经变成僵尸线程，一旦解锁，可能被其他线程解锁运行，因此需要先找到新的栈
    // 查看线程列表是否有可以切换的线程，换栈，解锁
    uint64_t unlock = 0;
    while (!atomic_compare_exchange_weak_explicit(&schedulable_threads_lock, &unlock, 1, memory_order_acquire, memory_order_relaxed))
        unlock = 0;
    struct Thread *const new_thread = schedulable_threads;
    if (new_thread == NULL) {
        atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_release);
        // 切换至专属Thread
        // 待补充
        abort();
    }
    CDL_DELETE2(schedulable_threads, new_thread, schedulable_threads_prev, schedulable_threads_next);

    atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_release);



    sti_spin_unlock(&mtx->spin_mtx, sti);
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
