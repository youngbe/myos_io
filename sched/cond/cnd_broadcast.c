#include <threads.h>

static struct Thread * __attribute__((noinline)) cnd_broadcast_part1(cnd_t *const cond, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    struct Thread *ret;
    if (is_sti)
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&cond->spin_mtx, p_spin_mutex_member);
    {
        ret = cond->threads;
        if (ret != NULL)
            cond->threads = NULL;
    }
    spin_unlock(&cond->spin_mtx, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);
    if (is_sti)
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
    return ret;
}

static struct Thread * __attribute__((noinline)) cnd_broadcast_part2(struct Mutex *const mtx, struct Thread *const thread, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    struct Thread *old_owner = NULL;
    if (is_sti)
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&mtx->spin_mtx, p_spin_mutex_member);
    {
        if (!atomic_compare_exchange_strong_explicit(&mtx->owner, &old_owner, thread, memory_order_relaxed, memory_order_relaxed)) {
            struct Thread *fake_blocked_threads = mtx->blocked_threads;
            DL_APPEND(fake_blocked_threads, thread);
            mtx->blocked_threads = fake_blocked_threads;
        }
    }
    spin_unlock(&mtx->spin_mtx, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);
    if (is_sti)
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
    return old_owner;
}

int cnd_broadcast(cnd_t *const cond)
{
    struct Spin_Mutex_Member spin_mutex_member;
    uint64_t rflags;
    __asm__ volatile (
            "pushfq\n\t"
            "popq   %0"
            :"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    const uint32_t is_sti = rflags & 512;
    spin_mutex_member_init(&spin_mutex_member);
    struct Thread *const threads = cnd_broadcast_part1(cond, is_sti, &spin_mutex_member);
    if (threads == NULL)
        return thrd_success;
    struct Thread *new_schedulable_threads = NULL;
    size_t new_schedulable_threads_num = 0;
    {
        struct Thread *thread, *tmp;
        DL_FOREACH_SAFE(threads, thread, tmp)
        {
            struct Mutex *const mtx = (struct Mutex *)(uintptr_t)thread->temp0;
            if (mtx != NULL) {
                // 在目前的实现中，不等待解锁也没有问题
                // while (mtx->owner == thread) {}
                struct Thread *cur_owner = NULL;
                if (atomic_compare_exchange_strong_explicit(&mtx->owner, &cur_owner, thread, memory_order_relaxed, memory_order_relaxed)) {
                    DL_APPEND(new_schedulable_threads, thread);
                    ++new_schedulable_threads_num;
                }
                else {
                    spin_mutex_member_init(&spin_mutex_member);
                    if (cnd_broadcast_part2(mtx, thread, is_sti, &spin_mutex_member) == NULL) {
                        DL_APPEND(new_schedulable_threads, thread);
                        ++new_schedulable_threads_num;
                    }
                }
            } else {
                DL_APPEND(new_schedulable_threads, thread);
                ++new_schedulable_threads_num;
            }
        }
    }
    if (new_schedulable_threads_num > 1) {
        spin_mutex_member_init(&spin_mutex_member);
        set_threads_schedulable(new_schedulable_threads, new_schedulable_threads_num, is_sti, &spin_mutex_member);
    } else if (new_schedulable_threads_num == 1) {
        spin_mutex_member_init(&spin_mutex_member);
        set_thread_schedulable(new_schedulable_threads, is_sti, &spin_mutex_member);
    }
    return thrd_success;
}
