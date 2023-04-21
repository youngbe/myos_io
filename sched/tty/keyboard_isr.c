#include "tty-internal.h"

    __attribute__((interrupt))
void keyboard_isr(void *)
{
    const char c = inb(0x60);
    size_t temp_used = atomic_load_explicit(&keyboard_buf_used, memory_order_relaxed);
    while (true) {
        if (temp_used == KEYBOARD_BUF_SIZE)
            return;
        if (atomic_compare_exchange_strong_explicit(&keyboard_buf_used, &temp_used, temp_used + 1, memory_order_acquire, memory_order_relaxed))
            break;
        __asm__ volatile ("pause");
    }
    if (temp_used == 0) {
        // 叫起床
        struct Thread *temp_keyboard_thread = NULL;
        if (!atomic_compare_exchange_strong_explicit(&keyboard_thread, &temp_keyboard_thread, (void *)1, memory_order_relaxed, memory_order_relaxed)) {
            struct Spin_Mutex_Member spin_mutex_member;
            spin_mutex_member_init(&spin_mutex_member);
            set_thread_schedulable_inline(temp_keyboard_thread, false, &spin_mutex_member);
        }
    }
    // 需要保证 keyboard_buf_used 已经写入
    const uint16_t temp_i = atomic_fetch_add_explicit(&keyboard_buf_ii, 1, memory_order_relaxed);
    atomic_store_explicit(&keyboard_buf[temp_i], c, memory_order_relaxed);
    atomic_signal_fence(memory_order_release);
    wrmsr_volatile_seq(0x80b, 0);
}
