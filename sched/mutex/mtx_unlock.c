#include <threads.h>

#include "sched-internal.h"
#include "thrd_current.h"
#include "mcs_spin.h"

#include <io.h>

#include <utlist.h>

#include <stdatomic.h>

static struct Thread *__attribute__((noinline)) mtx_unlock_part1(mtx_t*const mtx, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    if (is_sti) {
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
        atomic_signal_fence(memory_order_acquire);
    }

    spin_lock(&mtx->spin_mtx, p_spin_mutex_member);
    struct Thread *const ret = mtx->blocked_threads;
    if (ret == NULL)
        atomic_store_explicit(&mtx->owner, NULL, memory_order_relaxed);
    else {
        thrd_t fake_blocked_threads = ret;
        DL_DELETE(fake_blocked_threads, ret);
        mtx->blocked_threads = fake_blocked_threads;
    }
    spin_unlock(&mtx->spin_mtx, p_spin_mutex_member);
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
    }
    return ret;
}

int mtx_unlock(mtx_t*const mtx)
{
    thrd_t mtx_owner;
    {
        const thrd_t current_thread = thrd_currentx();
        mtx_owner = atomic_load_explicit(&mtx->owner, memory_order_relaxed);
        if (mtx_owner != current_thread)
            return thrd_error;
        if (mtx->count > 1) {
            atomic_signal_fence(memory_order_release);
            --mtx->count;
            return thrd_success;
        }
    }
    uint64_t rflags;
    __asm__ volatile(
            "pushfq\n\t"
            "popq   %0"
            :"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    const uint32_t is_sti = rflags & 512;
    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    mtx_owner = mtx_unlock_part1(mtx, is_sti, &spin_mutex_member);
    if (mtx_owner != NULL) {
        atomic_store_explicit(&mtx->owner, mtx_owner, memory_order_relaxed);
        spin_mutex_member_init(&spin_mutex_member);
        set_thread_schedulable(mtx_owner, is_sti, &spin_mutex_member);
    }
    return thrd_success;
}
