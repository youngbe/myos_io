#include "tty-internal.h"

#include "mcs_spin.h"
#include "sched-internal.h"
#include "thrd_current.h"

#include <io.h>

volatile _Atomic(uint16_t) keyboard_buf[KEYBOARD_BUF_SIZE] = {0};
volatile _Atomic(uint32_t) keyboard_buf_used = 0;
static volatile _Atomic(uint32_t) keyboard_buf_ii = 0;
// keyboard_buf_oi 在 keyboard 线程的局部变量
volatile _Atomic(struct Thread *) keyboard_sleeping_thread = NULL;

// 只有一个扫描码时，使用这个map
static const uint16_t ps2_set1_map[UINT8_MAX] = {
    [0x01] = (ESC << 1) | 1,
    [0x02] = (_1 << 1) | 1,
    [0x03] = (_2 << 1) | 1,
    [0x04] = (_3 << 1) | 1,
    [0x05] = (_4 << 1) | 1,
    [0x06] = (_5 << 1) | 1,
    [0x07] = (_6 << 1) | 1,
    [0x08] = (_7 << 1) | 1,
    [0x09] = (_8 << 1) | 1,
    [0x0a] = (_9 << 1) | 1,
    [0x0b] = (_0 << 1) | 1,
    [0x0c] = (SUB << 1) | 1,
    [0x0d] = (EQU << 1) | 1,
    [0x0e] = (BACK << 1) | 1,
    [0x0f] = (TAB << 1) | 1,
    [0x10] = (Q << 1) | 1,
    [0x11] = (W << 1) | 1,
    [0x12] = (E << 1) | 1,
    [0x13] = (R << 1) | 1,
    [0x14] = (T << 1) | 1,
    [0x15] = (Y << 1) | 1,
    [0x16] = (U << 1) | 1,
    [0x17] = (I << 1) | 1,
    [0x18] = (O << 1) | 1,
    [0x19] = (P << 1) | 1,
    [0x1a] = (OPEN_BRACKET << 1) | 1,
    [0x1b] = (CLOSE_BRACKET << 1) | 1,
    [0x1c] = (ENTER << 1) | 1,
    [0x1d] = (LEFT_CTRL << 1) | 1,
    [0x1e] = (A << 1) | 1,
    [0x1f] = (S << 1) | 1,
    [0x20] = (D << 1) | 1,
    [0x21] = (F << 1) | 1,
    [0x22] = (G << 1) | 1,
    [0x23] = (H << 1) | 1,
    [0x24] = (J << 1) | 1,
    [0x25] = (K << 1) | 1,
    [0x26] = (L << 1) | 1,
    [0x27] = (SEMICOLON << 1) | 1,
    [0x28] = (QUOTE << 1) | 1,
    [0x29] = (BACK_QUOTE << 1) | 1,
    [0x2a] = (LEFT_SHIFT << 1) | 1,
    [0x2b] = (BACKSLASH << 1) | 1,
    [0x2c] = (Z << 1) | 1,
    [0x2d] = (X << 1) | 1,
    [0x2e] = (C << 1) | 1,
    [0x2f] = (V << 1) | 1,
    [0x30] = (B << 1) | 1,
    [0x31] = (N << 1) | 1,
    [0x32] = (M << 1) | 1,
    [0x33] = (COMMA << 1) | 1,
    [0x34] = (DOT << 1) | 1,
    [0x35] = (SLASH << 1) | 1,
    [0x36] = (RIGHT_SHIFT << 1) | 1,
    [0x37] = (__ASTERISK << 1) | 1,
    [0x38] = (LEFT_ALT << 1) | 1,
    [0x39] = (SPACE << 1) | 1,
    [0x3a] = (CAP << 1) | 1,
    [0x3b] = (F1 << 1) | 1,
    [0x3c] = (F2 << 1) | 1,
    [0x3d] = (F3 << 1) | 1,
    [0x3e] = (F4 << 1) | 1,
    [0x3f] = (F5 << 1) | 1,
    [0x40] = (F6 << 1) | 1,
    [0x41] = (F7 << 1) | 1,
    [0x42] = (F8 << 1) | 1,
    [0x43] = (F9 << 1) | 1,
    [0x44] = (F10 << 1) | 1,
    [0x45] = (__NUM << 1) | 1,
    [0x46] = (SCRLK << 1) | 1,
    [0x47] = (__7 << 1) | 1,
    [0x48] = (__8 << 1) | 1,
    [0x49] = (__9 << 1) | 1,
    [0x4a] = (__SUB << 1) | 1,
    [0x4b] = (__4 << 1) | 1,
    [0x4c] = (__5 << 1) | 1,
    [0x4d] = (__6 << 1) | 1,
    [0x4e] = (__ADD << 1) | 1,
    [0x4f] = (__1 << 1) | 1,
    [0x50] = (__2 << 1) | 1,
    [0x51] = (__3 << 1) | 1,
    [0x52] = (__0 << 1) | 1,
    [0x53] = (__DOT << 1) | 1,
    [0x57] = (F11 << 1) | 1,
    [0x58] = (F12 << 1) | 1,
    [0x80 + 0x01] = ESC << 1,
    [0x80 + 0x02] = _1 << 1,
    [0x80 + 0x03] = _2 << 1,
    [0x80 + 0x04] = _3 << 1,
    [0x80 + 0x05] = _4 << 1,
    [0x80 + 0x06] = _5 << 1,
    [0x80 + 0x07] = _6 << 1,
    [0x80 + 0x08] = _7 << 1,
    [0x80 + 0x09] = _8 << 1,
    [0x80 + 0x0a] = _9 << 1,
    [0x80 + 0x0b] = _0 << 1,
    [0x80 + 0x0c] = SUB << 1,
    [0x80 + 0x0d] = EQU << 1,
    [0x80 + 0x0e] = BACK << 1,
    [0x80 + 0x0f] = TAB << 1,
    [0x80 + 0x10] = Q << 1,
    [0x80 + 0x11] = W << 1,
    [0x80 + 0x12] = E << 1,
    [0x80 + 0x13] = R << 1,
    [0x80 + 0x14] = T << 1,
    [0x80 + 0x15] = Y << 1,
    [0x80 + 0x16] = U << 1,
    [0x80 + 0x17] = I << 1,
    [0x80 + 0x18] = O << 1,
    [0x80 + 0x19] = P << 1,
    [0x80 + 0x1a] = OPEN_BRACKET << 1,
    [0x80 + 0x1b] = CLOSE_BRACKET << 1,
    [0x80 + 0x1c] = ENTER << 1,
    [0x80 + 0x1d] = LEFT_CTRL << 1,
    [0x80 + 0x1e] = A << 1,
    [0x80 + 0x1f] = S << 1,
    [0x80 + 0x20] = D << 1,
    [0x80 + 0x21] = F << 1,
    [0x80 + 0x22] = G << 1,
    [0x80 + 0x23] = H << 1,
    [0x80 + 0x24] = J << 1,
    [0x80 + 0x25] = K << 1,
    [0x80 + 0x26] = L << 1,
    [0x80 + 0x27] = SEMICOLON << 1,
    [0x80 + 0x28] = QUOTE << 1,
    [0x80 + 0x29] = BACK_QUOTE << 1,
    [0x80 + 0x2a] = LEFT_SHIFT << 1,
    [0x80 + 0x2b] = BACKSLASH << 1,
    [0x80 + 0x2c] = Z << 1,
    [0x80 + 0x2d] = X << 1,
    [0x80 + 0x2e] = C << 1,
    [0x80 + 0x2f] = V << 1,
    [0x80 + 0x30] = B << 1,
    [0x80 + 0x31] = N << 1,
    [0x80 + 0x32] = M << 1,
    [0x80 + 0x33] = COMMA << 1,
    [0x80 + 0x34] = DOT << 1,
    [0x80 + 0x35] = SLASH << 1,
    [0x80 + 0x36] = RIGHT_SHIFT << 1,
    [0x80 + 0x37] = __ASTERISK << 1,
    [0x80 + 0x38] = LEFT_ALT << 1,
    [0x80 + 0x39] = SPACE << 1,
    [0x80 + 0x3a] = CAP << 1,
    [0x80 + 0x3b] = F1 << 1,
    [0x80 + 0x3c] = F2 << 1,
    [0x80 + 0x3d] = F3 << 1,
    [0x80 + 0x3e] = F4 << 1,
    [0x80 + 0x3f] = F5 << 1,
    [0x80 + 0x40] = F6 << 1,
    [0x80 + 0x41] = F7 << 1,
    [0x80 + 0x42] = F8 << 1,
    [0x80 + 0x43] = F9 << 1,
    [0x80 + 0x44] = F10 << 1,
    [0x80 + 0x45] = __NUM << 1,
    [0x80 + 0x46] = SCRLK << 1,
    [0x80 + 0x47] = __7 << 1,
    [0x80 + 0x48] = __8 << 1,
    [0x80 + 0x49] = __9 << 1,
    [0x80 + 0x4a] = __SUB << 1,
    [0x80 + 0x4b] = __4 << 1,
    [0x80 + 0x4c] = __5 << 1,
    [0x80 + 0x4d] = __6 << 1,
    [0x80 + 0x4e] = __ADD << 1,
    [0x80 + 0x4f] = __1 << 1,
    [0x80 + 0x50] = __2 << 1,
    [0x80 + 0x51] = __3 << 1,
    [0x80 + 0x52] = __0 << 1,
    [0x80 + 0x53] = __DOT << 1,
    [0x80 + 0x57] = F11 << 1,
    [0x80 + 0x58] = F12 << 1,
};
// 当第一个扫描码 == 0xE0 时，第二个扫描码使用这个map
static const uint16_t ps2_set1_map2[UINT8_MAX] = {
    [0x1c] = (__ENTER << 1) | 1,
    [0x1d] = (RIGHT_CTRL << 1) | 1,
    [0x35] = (__SLASH << 1) | 1,
    [0x38] = (RIGHT_ALT << 1) | 1,
    [0x47] = (HOME << 1) | 1,
    [0x48] = (CURSOR_UP << 1) | 1,
    [0x49] = (PAGE_UP << 1) | 1,
    [0x4b] = (CURSOR_LEFT << 1) | 1,
    [0x4d] = (CURSOR_RIGHT << 1) | 1,
    [0x4f] = (END << 1) | 1,
    [0x50] = (CURSOR_DOWN << 1) | 1,
    [0x51] = (PAGE_DOWN << 1) | 1,
    [0x52] = (INSERT << 1) | 1,
    [0x53] = (DEL << 1) | 1,
    [0x80 + 0x1c] = __ENTER << 1,
    [0x80 + 0x1d] = RIGHT_CTRL << 1,
    [0x80 + 0x35] = __SLASH << 1,
    [0x80 + 0x38] = RIGHT_ALT << 1,
    [0x80 + 0x47] = HOME << 1,
    [0x80 + 0x48] = CURSOR_UP << 1,
    [0x80 + 0x49] = PAGE_UP << 1,
    [0x80 + 0x4b] = CURSOR_LEFT << 1,
    [0x80 + 0x4d] = CURSOR_RIGHT << 1,
    [0x80 + 0x4f] = END << 1,
    [0x80 + 0x50] = CURSOR_DOWN << 1,
    [0x80 + 0x51] = PAGE_DOWN << 1,
    [0x80 + 0x52] = INSERT << 1,
    [0x80 + 0x53] = DEL << 1,
};

static int get_scan_code(uint8_t *const scan_code)
{
    if ((inb(0x64) & 1) == 0)
        return -1;
    *scan_code = inb(0x60);
    return 0;
}

static inline __attribute__((always_inline, no_caller_saved_registers)) void
keyboard_isr_wrap(void *rsp)
{
    uint8_t scan_code;
    if (get_scan_code(&scan_code) != 0)
        goto label_abort;

    uint16_t key_code;
    if (scan_code == 0xE0) {
        if (get_scan_code(&scan_code) != 0)
            goto label_abort;
        if (scan_code == 0x2A) {
            if (get_scan_code(&scan_code) != 0 || scan_code != 0xE0)
                goto label_abort;
            if (get_scan_code(&scan_code) != 0 || scan_code != 0x37)
                goto label_abort;
            key_code = (PRTSC << 1) | 1;
        } else if (scan_code == 0xB7) {
            if (get_scan_code(&scan_code) != 0 || scan_code != 0xE0)
                goto label_abort;
            if (get_scan_code(&scan_code) != 0 || scan_code != 0xAA)
                goto label_abort;
            key_code = PRTSC << 1;
        } else
            key_code = ps2_set1_map2[scan_code];
    } else if (scan_code == 0xE1) {
        if (get_scan_code(&scan_code) != 0 || scan_code != 0x1D)
            goto label_abort;
        if (get_scan_code(&scan_code) != 0 || scan_code != 0x45)
            goto label_abort;
        if (get_scan_code(&scan_code) != 0 || scan_code != 0xE1)
            goto label_abort;
        if (get_scan_code(&scan_code) != 0 || scan_code != 0x9D)
            goto label_abort;
        if (get_scan_code(&scan_code) != 0 || scan_code != 0xC5)
            goto label_abort;
        key_code = (PAUSE << 1) | 1;
    } else
        key_code = ps2_set1_map[scan_code];

    if ((inb(0x64) & 1) != 0)
        goto label_abort;
    if (key_code == 0)
        goto label_abort;

    uint32_t temp_used = atomic_load_explicit(&keyboard_buf_used, memory_order_relaxed);
    while (true) {
        if (temp_used == KEYBOARD_BUF_SIZE)
            return;
        if (atomic_compare_exchange_strong_explicit(&keyboard_buf_used, &temp_used, temp_used + 1, memory_order_acquire, memory_order_relaxed))
            break;
        __asm__ volatile ("pause");
    }
    struct Thread *current_thread = NULL;
    struct Thread *temp_keyboard_sleeping_thread = NULL;
    if (temp_used == 0) {
        // 叫起床
        if (!atomic_compare_exchange_strong_explicit(&keyboard_sleeping_thread, &temp_keyboard_sleeping_thread, (void *)1, memory_order_relaxed, memory_order_relaxed)) {
            if ((((uint64_t *)rsp)[1] & 0b11) != 0)
                current_thread = (void *)1;
            else
                current_thread = thrd_currentx();
            if (current_thread != NULL) {
                struct Spin_Mutex_Member spin_mutex_member;
                spin_mutex_member_init_interrupt(&spin_mutex_member);
                set_thread_schedulable_interrupt(temp_keyboard_sleeping_thread, &spin_mutex_member);
            } else
                atomic_fetch_sub_explicit(&idle_cores_num, 1, memory_order_relaxed);
        }
    }
    // 需要保证 keyboard_buf_used 已经写入
    // 保证叫起床已经发生
    const uint16_t temp_i = atomic_fetch_add_explicit(&keyboard_buf_ii, 1, memory_order_release) & (KEYBOARD_BUF_SIZE - 1);
    atomic_store_explicit(&keyboard_buf[temp_i], key_code, memory_order_relaxed);
    if (temp_keyboard_sleeping_thread == NULL || current_thread != NULL) {
        atomic_signal_fence(memory_order_release);
        wrmsr_volatile_seq_interrupt(0x80b, 0);
        return;
    } else {
        __asm__ volatile (
                "movq	%%gs:8, %%rsi\n\t"
                "jmp    switch_to_interrupt"
                :
                :"D"(temp_keyboard_sleeping_thread)
                :);
        __builtin_unreachable();
    }

label_abort:
    __asm__ volatile ("jmp abort");
    __builtin_unreachable();
}

    __attribute__((interrupt))
void keyboard_isr(void *rsp)
{
    keyboard_isr_wrap(rsp);
}
