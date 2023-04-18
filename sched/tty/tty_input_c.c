#include "tty-internal.h"

#include "sched-internal.h"

// 多个生产者，来自中断
// 并发且无锁，但是中断处于关闭状态(cli)
void tty_input_c(struct TTY *const tty, const char c)
{
    size_t temp_used = atomic_load_explicit(&tty->input_buf_used, memory_order_relaxed);
    while (true) {
        if (temp_used == 65536)
            return;
        if (atomic_compare_exchange_strong_explicit(&tty->input_buf_used, &temp_used, temp_used + 1, memory_order_relaxed, memory_order_relaxed))
            break;
        __asm__ volatile ("pause");
    }
    if (temp_used == 0) {
        // 叫起床
        struct Thread *temp_sleeping_thread = NULL;
        if (!atomic_compare_exchange_strong_explicit(&tty->sleeping_thread, &temp_sleeping_thread, (void *)1, memory_order_relaxed, memory_order_relaxed)) {
            struct Spin_Mutex_Member spin_mutex_member;
            spin_mutex_member_init(&spin_mutex_member);
            set_thread_schedulable_inline(temp_sleeping_thread, false, &spin_mutex_member);
        }
    }
    //assert(buf[input_i] == '\0');
    const uint16_t temp_i = atomic_fetch_add_explicit(&tty->input_ii, 1, memory_order_release);
    atomic_store_explicit(&tty->input_buf[temp_i], c, memory_order_release);
}
