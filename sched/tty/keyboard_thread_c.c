#include "tty-internal.h"
#include "sched-internal.h"

#include "thrd_current.h"


#include <io.h>

#include <stdbool.h>
#include <stdlib.h>

#define KEYBOARD_THREAD_BUF_SIZE 65536

static bool keymap[KEY_NUM + 1] = {false};
static const char to_ascii[KEY_NUM + 1] = {
    [_1] = '1',
    [_2] = '2',
    [_3] = '3',
    [_4] = '4',
    [_5] = '5',
    [_6] = '6',
    [_7] = '7',
    [_8] = '8',
    [_9] = '9',
    [_0] = '0',
    [__1] = '1',
    [__2] = '2',
    [__3] = '3',
    [__4] = '4',
    [__5] = '5',
    [__6] = '6',
    [__7] = '7',
    [__8] = '8',
    [__9] = '9',
    [__0] = '0',
    [A] = 'a',
    [B] = 'b',
    [C] = 'c',
    [D] = 'd',
    [E] = 'e',
    [F] = 'f',
    [G] = 'g',
    [H] = 'h',
    [I] = 'i',
    [J] = 'j',
    [K] = 'k',
    [L] = 'l',
    [M] = 'm',
    [N] = 'n',
    [O] = 'o',
    [P] = 'p',
    [Q] = 'q',
    [R] = 'r',
    [S] = 's',
    [T] = 't',
    [U] = 'u',
    [V] = 'v',
    [W] = 'w',
    [X] = 'x',
    [Y] = 'y',
    [Z] = 'z',
    [BACK_QUOTE] = '`',
    [SUB] = '-',
    [EQU] = '=',
    [TAB] = '\t',
    [OPEN_BRACKET] = '[',
    [CLOSE_BRACKET] = ']',
    [BACKSLASH] = '\\',
    [SEMICOLON] = ';',
    [QUOTE] = '\'',
    [ENTER] = '\n',
    [COMMA] = ',',
    [DOT] = '.',
    [SLASH] = '/',
    [SPACE] = ' ',
    [__SLASH] = '/',
    [__ASTERISK] = '*',
    [__SUB] = '-',
    [__ADD] = '+',
    [__ENTER] = '\n',
    [__DOT] = '.',
};
static const char to_ascii_shift[KEY_NUM + 1];
static const char to_ascii_cap[KEY_NUM + 1];
static const char to_ascii_cap_shift[KEY_NUM + 1];

int keyboard_thread(void *)
{
    uint64_t fsbase;
    uint32_t mxcsr;
    uint16_t x87_cw;
    __asm__ volatile (
            "rdfsbase   %0"
            :"=r"(fsbase), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    __asm__ volatile (
            "stmxcsr    %0"
            :"=m"(mxcsr), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    __asm__ volatile (
            "fstcw      %0"
            :"=m"(x87_cw), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    /*
    __asm__ volatile (
            ""
            :
            :
            :"rbx", "rbp", "r12", "r13", "r14", "r15",
            "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22", "ymm23", "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29", "ymm30", "ymm31",
            "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29", "zmm30", "zmm31"
            );
            */

    struct Thread *const current_thread = thrd_currentx();
    size_t last_used;
    size_t keyboard_buf_oi = 0;

    goto label_in;
    while (true) {
        if (last_used == 1) {
label_in:
            ;
            // need sleep
            void *temp;
            __asm__ volatile (
                    "leaq   .Lwake_up(%%rip), %0"
                    :"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                    :
                    :);
            __asm__ volatile (
                    "pushfq\n\t"
                    "cli"
                    :"+m"(__not_exist_global_sym_for_asm_seq)
                    :"S"(current_thread)//, "D"(current_thread)
                    :);
            atomic_signal_fence(memory_order_acquire);
            __asm__ volatile (
                    "movq   %%rsp, %0"
                    :"+m"(current_thread->rsp)
                    :
                    :);
            current_thread->return_hook = temp;
            struct Thread *expected_thread = NULL;
            if (atomic_compare_exchange_strong_explicit(&keyboard_sleeping_thread, &expected_thread, current_thread, memory_order_release, memory_order_relaxed)) {
                // 不能再使用栈
                struct Proc *temp_current_proc = (struct Proc *)((uintptr_t)current_thread->proc & -2);
                __asm__ volatile (
                        "rdgsbase   %%rsp\n\t"
                        "addq       $65520, %%rsp\n\t"
                        "jmp        switch_to_empty\n"
                        ".Lwake_up:\n\t"
                        "popfq"
                        :"+D"(temp_current_proc), "+m"(__not_exist_global_sym_for_asm_seq)
                        :
                        :"cc", "rax", "rbx", "rcx", "rdx", "rsi", "rbp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                        "st", "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
                        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31",
                        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22", "ymm23", "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29", "ymm30", "ymm31",
                        "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29", "zmm30", "zmm31"
                        );
            } else {
                __asm__ volatile (
                        "sti\n\t"
                        "addq   $8, %%rsp"
                        :
                        :
                        :);
            }
            //assert(keyboard_sleeping_thread != NULL);
            atomic_store_explicit(&keyboard_sleeping_thread, NULL, memory_order_relaxed);
        }
        //assert(keyboard_buf_used >= 1);
        uint16_t key_codes[KEYBOARD_THREAD_BUF_SIZE];
        size_t key_codes_num = 1;

        {
            uint16_t temp;
            while ((temp = atomic_load_explicit(&keyboard_buf[keyboard_buf_oi], memory_order_relaxed)) == '\0')
                __asm__ volatile ("pause":::);
            key_codes[0] = temp;
        }
        atomic_store_explicit(&keyboard_buf[keyboard_buf_oi++], 0, memory_order_relaxed);
        // 使用 memory_order_release
        // 保证 keyboard_buf 0 已写入，sleeping_thread已写入
        last_used = atomic_fetch_sub_explicit(&keyboard_buf_used, 1, memory_order_release);
        keyboard_buf_oi &= KEYBOARD_BUF_SIZE - 1;
        while (true) {
            const uint16_t temp_code = atomic_load_explicit(&keyboard_buf[keyboard_buf_oi], memory_order_relaxed);
            if (temp_code == 0)
                break;
            if (last_used == 1)
                atomic_store_explicit(&keyboard_sleeping_thread, NULL, memory_order_relaxed);
            atomic_store_explicit(&keyboard_buf[keyboard_buf_oi++], 0, memory_order_relaxed);
            // 使用 memory_order_release
            // 保证 keyboard_buf 0 已写入，sleeping_thread已写入
            last_used = atomic_fetch_sub_explicit(&keyboard_buf_used, 1, memory_order_release);
            keyboard_buf_oi &= KEYBOARD_BUF_SIZE - 1;
            key_codes[key_codes_num++] = temp_code;
            if (key_codes_num == KEYBOARD_THREAD_BUF_SIZE)
                break;
        }

        // 将获取到的键盘事件数组转化为等效的 偏移ascii数组（字符串）
        // 偏移数组的含义：先输入 back_num 个退格健，再输入数组
        char c[KEYBOARD_THREAD_BUF_SIZE];
        size_t c_num = 0;
        size_t back_num = 0;
        static bool is_cap = false;
        for (size_t i = 0; i < key_codes_num; ++i) {
            // 按钮
            const uint16_t key = key_codes[i] >> 1;
            const bool is_press = (key_codes[i] & 1) == 1;
            if (key == CAP && is_press && !keymap[CAP])
                // 如果是按下大写的情况
                is_cap = !is_cap;

            // 更新每个按键的按下/释放情况
            keymap[key] = is_press;
            if (!is_press)
                // 如果仅是按键释放的话，不用做什么
                continue;

            if (key == BACK && is_press) {
                // backspace按下
                if (c_num > 0)
                    --c_num;
                else
                    ++back_num;
                continue;
            }

            char temp_c;
            if (is_cap && (keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
                temp_c = to_ascii_cap_shift[key];
            else if (!is_cap && (keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
                temp_c = to_ascii_shift[key];
            else if (is_cap && !(keymap[LEFT_SHIFT] || keymap[RIGHT_SHIFT]))
                temp_c = to_ascii_cap[key];
            else
                temp_c = to_ascii[key];
            if (temp_c != '\0')
                c[c_num++] = temp_c;
        }


        if (c_num == 0 && back_num == 0)
            continue;
        // handle c
        struct TTY *const tty = current_tty;
        if (mtx_lock(&tty->read_mtx) != thrd_success)
            abort();
        const size_t old_read_buf_visible = tty->read_buf_visible;
        const size_t old_read_buf_used = tty->read_buf_used;
        if (old_read_buf_used - old_read_buf_visible < back_num)
            back_num = old_read_buf_used - old_read_buf_visible;
        if (back_num != 0) {
            char temp[back_num * 3];
            for (size_t i = 0; i < back_num; ++i) {
                temp[i * 3] = '\b';
                temp[i * 3 + 1] = ' ';
                temp[i * 3 + 2] = '\b';
            }
            if (c_num != 0) {
                struct iovec iovs[2] = {
                    { .iov_base = temp, .iov_len = back_num * 3 },
                    { .iov_base = c, .iov_len = c_num }
                };
                tty->writev(NULL, iovs, 2);
            } else
                tty->write(NULL, temp, back_num * 3);
        } else if (c_num != 0)
            tty->write(NULL, c, c_num);
        else
            goto label_unlock;
        {
            const size_t read_buf_used = old_read_buf_used - back_num;
            size_t read_buf_ii = tty->read_buf_ii;
            if (back_num != 0) {
                if (read_buf_ii >= back_num)
                    read_buf_ii -= back_num;
                else
                    read_buf_ii = TTY_READ_BUF_SIZE - (back_num - read_buf_ii);
            }
            const size_t save_num = c_num > TTY_READ_BUF_SIZE - read_buf_used ? TTY_READ_BUF_SIZE - read_buf_used : c_num;
            if (save_num != 0) {
                size_t new_visible = 0;
                if (read_buf_used + save_num >= TTY_READ_BUF_VISIBLE_THRESHOLD)
                    new_visible = read_buf_used + save_num;
                for (size_t i = 0; i < save_num; ++i) {
                    tty->read_buf[read_buf_ii++] = c[i];
                    read_buf_ii %= TTY_READ_BUF_SIZE;
                    if (c[i] == '\n') {
                        if (read_buf_used + i + 1 > new_visible)
                            new_visible = read_buf_used + i + 1;
                    }
                }
                if (new_visible > 0) {
                    if (old_read_buf_visible == 0)
                        if (cnd_broadcast(&tty->read_cnd) != thrd_success)
                            abort();
                    tty->read_buf_visible = new_visible;
                }
            }
            tty->read_buf_ii = read_buf_ii; 
            tty->read_buf_used = read_buf_used + save_num;
        }
label_unlock:
        if (mtx_unlock(&tty->read_mtx) != thrd_success)
            abort();
    }

    __asm__ volatile (
            "wrfsbase   %1"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"r"(fsbase)
            :);
    __asm__ volatile (
            "stmxcsr    %1"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(mxcsr)
            :);
    __asm__ volatile (
            "fstcw      %1"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"m"(x87_cw)
            :);
    return 0;
}
