#pragma once

#include "mcs_spin.h"
#include "sched-internal.h"

#include <io.h>

#include <utlist.h>

static void __attribute__((noinline)) set_thread_schedulablex(struct Thread *const new_thread, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    if (is_sti) {
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
        // spin_lock 只有acquire属性，为了防止cli被移动至上锁之后，这里需要再添加一个fence
        atomic_signal_fence(memory_order_acquire);
    }
    spin_lock(&schedulable_threads_lock, p_spin_mutex_member);
    atomic_fetch_add_explicit(&schedulable_threads_num, 1, memory_order_acquire);
    struct Thread *const schedulable_threads_bak = schedulable_threads;
    struct Thread *fake_schedulable_threads = schedulable_threads_bak;
    CDL_APPEND(fake_schedulable_threads, new_thread);
    schedulable_threads = fake_schedulable_threads;
    spin_unlock(&schedulable_threads_lock, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);
    if (atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed) > 0 && atomic_load_explicit(&idle_cores_num, memory_order_relaxed) > 0)
        wrmsr_volatile_seq(0x830, 35 | 0b000 << 8 | 1 << 14 | 0b11 << 18);
    atomic_fetch_add_explicit(&old_schedulable_threads_num, 1, memory_order_acq_rel);
    if (is_sti)
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
}
