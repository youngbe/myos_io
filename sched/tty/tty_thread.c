#include "tty-internal.h"
#include "sched-internal.h"

#include <sched.h>

#include <stdbool.h>
#include <stdlib.h>

int tty_thread(void *const in_tty)
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

    struct TTY *const tty = in_tty;
    struct Thread *const current_thread = thrd_currentx();
    size_t last_used;
    uint16_t input_buf_oi = 0;

    goto label_in;
    while (true) {
        if (last_used == 1) {
label_in:
            // need sleep
            __asm__ volatile (
                    "pushfq\n\t"
                    "cli"
                    :
                    :
                    :);
            __asm__ volatile (
                    "movq   %%rsp, %0"
                    :"+m"(current_thread->rsp)
                    :
                    :);
            __asm__ volatile (
                    "leaq   .Lwake_up(%%rip), %0"
                    :"=r"(current_thread->return_hook)
                    :
                    :);
            struct Thread *expected_thread = NULL;
            if (atomic_compare_exchange_strong_explicit(&tty->sleeping_thread, &expected_thread, current_thread, memory_order_release, memory_order_relaxed)) {
                // 不能再使用栈
                //struct Proc *temp_current_proc = current_proc;
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
                //assert(sleeping_thread != NULL);
            } else {
                __asm__ volatile (
                        "sti\n\t"
                        "addq   $8, %%rsp"
                        :
                        :
                        :);
            }
            atomic_store_explicit(&tty->sleeping_thread, NULL, memory_order_relaxed);
        }
        //assert(used >= 1);
        char c;

        while ((c = atomic_load_explicit(&tty->input_buf[input_buf_oi], memory_order_relaxed)) == '\0')
            __asm__ volatile ("pause":::);
        atomic_store_explicit(&tty->input_buf[input_buf_oi++], '\0', memory_order_relaxed);
        // 使用memory_order_release
        // 保证 input_buf '\0' 已写入，sleeping_thread已写入
        last_used = atomic_fetch_sub_explicit(&tty->input_buf_used, 1, memory_order_release);
        // handle c
        if (mtx_lock(&tty->read_mtx) != thrd_success)
            abort();
        if (tty->read_buf_used == TTY_READ_BUF_SIZE)
            goto label_continue;
        tty->read_buf[tty->read_buf_ii] = c;
        tty->read_buf_ii = (tty->read_buf_ii + 1) % TTY_READ_BUF_SIZE;
        if (++tty->read_buf_used >= TTY_READ_BUF_VISIBLE_THRESHOLD || c == '\n') {
            if (tty->read_buf_visible == 0)
                if (cnd_broadcast(&tty->read_cnd) != thrd_success)
                    abort();
            tty->read_buf_visible = tty->read_buf_used;
        }
label_continue:
        if (mtx_unlock(&tty->read_mtx) != thrd_success)
            abort();
    }
}
