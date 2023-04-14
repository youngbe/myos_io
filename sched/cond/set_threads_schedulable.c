#include "mcs_spin.h"
#include "sched-internal.h"

#include <io.h>

#include <utlist.h>

void __attribute__((noinline)) set_threads_schedulable(struct Thread *const threads, const size_t num, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    if (threads == NULL || num == 0)
        __builtin_unreachable();
    if (is_sti)
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&schedulable_threads_lock, p_spin_mutex_member);
    {
        atomic_fetch_add_explicit(&schedulable_threads_num, num, memory_order_acquire);
        struct Thread *fake_schedulable_threads = schedulable_threads;
        DL_CONCAT(fake_schedulable_threads, threads);
        schedulable_threads = fake_schedulable_threads;
    }
    spin_unlock(&schedulable_threads_lock, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);
    if (atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed) > 0 && atomic_load_explicit(&idle_cores_num, memory_order_relaxed) > 0)
        wrmsr_volatile_seq(0x830, 35 | 0b000 << 8 | 1 << 14 | 0b11 << 18);
    atomic_fetch_add_explicit(&old_schedulable_threads_num, num, memory_order_acq_rel);
    if (is_sti)
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
}
