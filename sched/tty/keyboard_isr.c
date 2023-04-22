#include "tty-internal.h"

#include "mcs_spin.h"
#include "sched-internal.h"

#include <io.h>

volatile atomic_char keyboard_buf[KEYBOARD_BUF_SIZE] = {0};
volatile atomic_size_t keyboard_buf_used = 0;
// keyboard_buf_oi 在 keyboard 线程的局部变量
volatile _Atomic(struct Thread *) keyboard_sleeping_thread = NULL;


static volatile atomic_size_t keyboard_buf_ii = 0;
static inline __attribute__((always_inline, no_caller_saved_registers)) void
keyboard_isr_wrap(void)
{
    const char c = inb_interrupt(0x60);
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
        struct Thread *temp_keyboard_sleeping_thread = NULL;
        if (!atomic_compare_exchange_strong_explicit(&keyboard_sleeping_thread, &temp_keyboard_sleeping_thread, (void *)1, memory_order_relaxed, memory_order_relaxed)) {
            struct Spin_Mutex_Member spin_mutex_member;
            spin_mutex_member_init_interrupt(&spin_mutex_member);
            set_thread_schedulable_interrupt(temp_keyboard_sleeping_thread, &spin_mutex_member);
        }
    }
    // 需要保证 keyboard_buf_used 已经写入
    // 保证叫起床已经发生
    const uint16_t temp_i = atomic_fetch_add_explicit(&keyboard_buf_ii, 1, memory_order_release) & (KEYBOARD_BUF_SIZE - 1);
    atomic_store_explicit(&keyboard_buf[temp_i], c, memory_order_relaxed);
    atomic_signal_fence(memory_order_release);
    wrmsr_volatile_seq_interrupt(0x80b, 0);
}

    __attribute__((interrupt))
void keyboard_isr(void *)
{
    keyboard_isr_wrap();
}
