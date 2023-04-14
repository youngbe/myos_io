#include <threads.h>

#include "sched-internal.h"
#include "mcs_spin.h"
#include "thrd_current.h"

#include <io.h>

#include <utlist.h>

extern struct Core_Data kernel_gs_base;

int cnd_wait(cnd_t *const cond, mtx_t *const mtx)
{
    const thrd_t current_thread = thrd_currentx();
    if (atomic_load_explicit(&mtx->owner, memory_order_relaxed) != current_thread)
        return thrd_error;
    const bool need_lock = mtx->count < 2;
    if (need_lock)
        current_thread->temp0 = (uintptr_t)mtx;
    else
        current_thread->temp0 = 0;
    struct Proc *current_proc;
    bool current_is_kernel;
    {
        const uintptr_t temp = (uintptr_t)current_thread->proc;
        current_is_kernel = temp & 1;
        current_proc = (struct Proc *)(uintptr_t)(temp & -2);
    }
    // 增加虚拟线程数以防止页表被释放
    // 这一步也是切换至空线程的步骤之一
    if (!current_is_kernel)
        atomic_fetch_add_explicit(&current_proc->threads_num, 1, memory_order_relaxed);
    atomic_signal_fence(memory_order_release);
    struct Spin_Mutex_Member *p_spin_mutex_member;
    // 保存上下文 && spin_mutex_init && cli
    {
        uint64_t temp;
        __asm__ volatile (
                "pushq      %%rbp\n\t"
                "pushq      %%r15\n\t"
                "pushq      %%r14\n\t"
                "pushq      %%r13\n\t"
                "pushq      %%r12\n\t"
                "pushq      %%rbx\n\t"
                "subq       $16, %%rsp\n\t"
                "fstcw      8(%%rsp)\n\t"
                "stmxcsr    (%%rsp)\n\t"
                "rdfsbase   %0\n\t"
                "pushq      %0\n\t"
                "pushfq\n\t"
                "subq       $24, %%rsp"
                :"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :"cc");
        __asm__ volatile (
                "movq   %%rsp, %0"
                :"=r"(p_spin_mutex_member), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        spin_mutex_member_init(p_spin_mutex_member);
        atomic_signal_fence(memory_order_release);
        __asm__ volatile (
                "cli\n\t"
                "swapgs\n\t"
                "rdgsbase   %0\n\t"
                "swapgs\n\t"
                "movq       %0, 16(%%rsp)"
                :"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        atomic_signal_fence(memory_order_acquire);
    }
    current_thread->rsp = (void *)((uintptr_t)p_spin_mutex_member + 16);
    current_thread->return_hook = &&label_wakeup;


    // 上锁
    // 睡
    // 解锁:从这里开始原栈不可用
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&cond->spin_mtx, p_spin_mutex_member);
    {
        struct Thread *fake_threads = cond->threads;
        DL_APPEND(fake_threads, current_thread);
        cond->threads = fake_threads;
    }
    spin_unlock(&cond->spin_mtx, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);

    // 借用空闲栈
    __asm__ volatile (
            "rdgsbase   %%rsp\n\t"
            "addq       $65520, %%rsp"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :
            :"cc");

    if (!need_lock) {
        __asm__ goto volatile (
                "jmp    switch_to_empty"
                :
                :"D"(current_proc)
                :
                :label_wakeup);
        __builtin_unreachable();
    }

    __asm__ volatile (
            "movq     %%rsp, %0"
            :"=r"(p_spin_mutex_member), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);

    spin_mutex_member_init(p_spin_mutex_member);
    struct Thread *new_owner;
    // 上锁
    // unlock
    // 解锁
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&mtx->spin_mtx, p_spin_mutex_member);
    {
        new_owner = mtx->blocked_threads;
        if (new_owner == NULL)
            atomic_store_explicit(&mtx->owner, NULL, memory_order_relaxed);
        else {
            struct Thread *fake_blocked_threads = new_owner;
            DL_DELETE(fake_blocked_threads, new_owner);
            mtx->blocked_threads = fake_blocked_threads;
        }
    }
    spin_unlock(&mtx->spin_mtx, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);

    // 如果没有解锁出新线程 find_hook()
    if (new_owner != NULL) {
        // 在unlock之后再存
        atomic_store_explicit(&mtx->owner, new_owner, memory_order_relaxed);
        __asm__ volatile goto ("jmp switch_to"::"D"(new_owner), "S"(current_proc)::label_wakeup);
        __builtin_unreachable();
    } else {
        __asm__ goto volatile (
                "jmp    switch_to_empty"
                :
                :"D"(current_proc)
                :
                :label_wakeup);
        __builtin_unreachable();
    }

    {
label_wakeup:
        __asm__ volatile (
                "popq       %%r12\n\t"
                "swapgs\n\t"
                "wrgsbase   %%r12\n\t"
                "swapgs\n\t"
                "popfq\n\t"
                "popq       %%r13\n\t"
                "wrfsbase   %%r13\n\t"
                "ldmxcsr    (%%rsp)\n\t"
                "fldcw      8(%%rsp)\n\t"
                "addq       $16, %%rsp\n\t"
                "popq       %%rbx\n\t"
                "popq       %%r12\n\t"
                "popq       %%r13\n\t"
                "popq       %%r14\n\t"
                "popq       %%r15\n\t"
                "popq       %%rbp"
                :
                :
                :"cc", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                "st", "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20", "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27", "xmm28", "xmm29", "xmm30", "xmm31",
                "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15", "ymm16", "ymm17", "ymm18", "ymm19", "ymm20", "ymm21", "ymm22", "ymm23", "ymm24", "ymm25", "ymm26", "ymm27", "ymm28", "ymm29", "ymm30", "ymm31",
                "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20", "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27", "zmm28", "zmm29", "zmm30", "zmm31"
                    );
        return thrd_success;
    }
}
