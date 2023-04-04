#include <threads.h>

#include "sched-internal.h"

#include <utlist.h>

#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>

static inline bool cli_spin_lock(uint64_t *const spin_mutex)
{
    bool is_sti;
    uint64_t rflags;
    __asm__ volatile (
            "pushfq\n\t"
            "popq   %0"
            :"=g"(rflags)
            :
            :"rsp");
    is_sti = (rflags & 0x0200) != 0;
    if (is_sti) {
        __asm__ volatile (
                "cli"
                :
                :
                :"memory");
    }
    uint64_t unlocked = 0;
    const uint64_t locked = 1;
    // we use memory_order_acq_rel because we need cli run before it
    while (!atomic_compare_exchange_weak_explicit((_Atomic uint64_t *)spin_mutex, &unlocked, locked, memory_order_acq_rel, memory_order_release))
        unlocked = 0;
    return is_sti;
}

static inline void sti_spin_unlock(uint64_t *const spin_mutex, const bool sti)
{
    // we use memory_order_acq_rel here because we need sti run after it
    // memory_order_acq_rel is disallow so we use memory_order_release + fence(memory_order_acq_rel)
    atomic_store_explicit((_Atomic uint64_t *)spin_mutex, 0, memory_order_release);
    atomic_signal_fence(memory_order_acq_rel);
    if (sti) {
        __asm__ volatile (
                "sti"
                :
                :
                :"memory");
    }
}

int __attribute__((noinline)) mtx_unlock(mtx_t*const mtx)
{
    const thrd_t current_thrd = thrd_current();
    thrd_t mtx_owner = atomic_load_explicit(&mtx->owner, memory_order_relaxed);
    if (mtx_owner != current_thrd)
        return thrd_error;
    if (mtx->count > 1) {
        --mtx->count;
        return thrd_success;
    }
    const bool sti = cli_spin_lock(&mtx->spin_mtx);
    mtx_owner = mtx->blocked_threads;
    //atomic_store_explicit(&mtx->owner, mtx_owner, memory_order_relaxed);
    //if (mtx_owner != NULL)
    //    CDL_DELETE(mtx->blocked_threads, mtx->blocked_threads);

    if (mtx_owner == NULL)
        atomic_store_explicit(&mtx->owner, NULL, memory_order_relaxed);
    else
        CDL_DELETE(mtx->blocked_threads, mtx->blocked_threads);

    sti_spin_unlock(&mtx->spin_mtx, sti);
    if (mtx_owner) {
        atomic_store_explicit(&mtx->owner, mtx_owner, memory_order_relaxed);
        atomic_signal_fence(memory_order_acq_rel);
        // 将mtx_owner放入调度队列
        // 待实现
        abort();
    }
    return thrd_success;
}
