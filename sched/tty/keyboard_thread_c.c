#include "tty-internal.h"
#include "sched-internal.h"

#include "thrd_current.h"


#include <sched.h>
#include <io.h>

#include <stdbool.h>
#include <stdlib.h>

#define KEYBOARD_THREAD_BUF_SIZE 65536

int keyboard_thread(void *)
{
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
                    :"r"(current_thread)
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
                        :"+D"(temp_current_proc)
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
        char c[KEYBOARD_THREAD_BUF_SIZE];
        size_t c_num = 1;

        {
            char temp_c;
            while ((temp_c = atomic_load_explicit(&keyboard_buf[keyboard_buf_oi], memory_order_relaxed)) == '\0')
                __asm__ volatile ("pause":::);
            c[0] = temp_c;
        }
        atomic_store_explicit(&keyboard_buf[keyboard_buf_oi++], '\0', memory_order_relaxed);
        // 使用 memory_order_release
        // 保证 keyboard_buf '\0' 已写入，sleeping_thread已写入
        last_used = atomic_fetch_sub_explicit(&keyboard_buf_used, 1, memory_order_release);
        keyboard_buf_oi &= KEYBOARD_BUF_SIZE - 1;
        while (last_used != 1) {
            const char temp_c = atomic_load_explicit(&keyboard_buf[keyboard_buf_oi], memory_order_relaxed);
            if (temp_c == '\0')
                break;
            atomic_store_explicit(&keyboard_buf[keyboard_buf_oi++], '\0', memory_order_relaxed);
            // 使用 memory_order_release
            // 保证 keyboard_buf '\0' 已写入，sleeping_thread已写入
            last_used = atomic_fetch_sub_explicit(&keyboard_buf_used, 1, memory_order_release);
            keyboard_buf_oi &= KEYBOARD_BUF_SIZE - 1;
            c[c_num++] = temp_c;
            if (c_num == KEYBOARD_THREAD_BUF_SIZE)
                break;
        }
        // handle c
        struct TTY *const tty = current_tty;
        if (mtx_lock(&tty->read_mtx) != thrd_success)
            abort();

        const size_t read_buf_used = tty->read_buf_used;
        const size_t save_num = c_num > TTY_READ_BUF_SIZE - read_buf_used ? TTY_READ_BUF_SIZE - read_buf_used : c_num;
        if (save_num != 0) {
            size_t new_visible = 0;
            if (read_buf_used + save_num >= TTY_READ_BUF_VISIBLE_THRESHOLD)
                new_visible = read_buf_used + save_num;
            size_t read_buf_ii = tty->read_buf_ii;
            for (size_t i = 0; i < save_num; ++i) {
                tty->read_buf[read_buf_ii++] = c[i];
                read_buf_ii %= TTY_READ_BUF_SIZE;
                if (c[i] == '\n') {
                    if (read_buf_used + i > new_visible)
                        new_visible = read_buf_used + i;
                }
            }
            tty->read_buf_ii = read_buf_ii; 
            if (new_visible > 0) {
                if (tty->read_buf_visible == 0)
                    if (cnd_broadcast(&tty->read_cnd) != thrd_success)
                        abort();
                tty->read_buf_visible = new_visible;
            }
            tty->read_buf_used = read_buf_used + save_num;
        }

        if (mtx_unlock(&tty->read_mtx) != thrd_success)
            abort();

        if (save_num != 0)
            tty->tty_write(NULL, c, save_num);
    }
    return 0;
}