#include "threads.h"
#include "sched-internal.h"

#include <stdint.h>
#include <stdatomic.h>

extern struct Core_Data kernel_gs_base;
int mtx_lock(struct Mutex *const mutex)
{
    struct Thread *current_thread;
    __asm__ volatile (""::"r"(&mutex->waiters):);
    {
        const struct Thread *const current_owner = *(void **)&mutex->owner;
        if (current_owner == NULL) {
            void *current_wait_end = *(void *volatile *)&mutex->wait_end;
            if (current_wait_end == NULL && atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_wait_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed)) {
                atomic_store_explicit(&mutex->owner, thrd_current_inline(), memory_order_relaxed);
                return thrd_success;
            }
            current_thread = thrd_current_inline();
        } else if (current_owner == (current_thread = thrd_current_inline())) {
            if (mutex->count == 0 || mutex->count == SIZE_MAX)
                return thrd_error;
            ++mutex->count;
            return thrd_success;
        }
    }

    void *const node = &current_thread->temp0;
    *(void **)node = NULL;

    // 增加虚拟线程数以防止页表被释放
    struct Proc *const current_proc = ({
            struct Proc *ret;
            const uintptr_t temp = current_thread->proc;
            if (temp & 1)
                ret = NULL;
            else {
                ret = (struct Proc *)temp;
                atomic_fetch_add_explicit(&ret->threads_num, 1, memory_order_relaxed);
            }
            ret;
            });
    // 保存上下文 && cli
    bool is_sti;
    {
        uint64_t temp;
        uint64_t rflags;
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
                "rdfsbase   %[temp]\n\t"
                "pushq      %[temp]"
                :[temp]"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        __asm__ volatile (
                "pushfq\n\t"
                "movq   (%%rsp), %[rflags]"
                :[rflags]"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        is_sti = rflags & 512;
        if (is_sti) {
            __asm__ volatile (
                    "cli"
                    :"+m"(__not_exist_global_sym_for_asm_seq)
                    :
                    :);
        }
        __asm__ volatile (
                "swapgs\n\t"
                "rdgsbase   %[temp]\n\t"
                "swapgs\n\t"
                "pushq      %[temp]\n\t"
                :[temp]"=r"(temp), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        __asm__ volatile (
                "movq   %%rsp, %0"
                :"=m"(current_thread->rsp), "+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    current_thread->return_hook = &&label_wakeup;


    void *current_wait_end = *(void *volatile *)&mutex->wait_end;
    if (current_wait_end == NULL) {
        if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_wait_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed)) {
label_fast_exit:
            // 成为了owner
            if (is_sti)
                __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
            atomic_store_explicit(&mutex->owner, current_thread, memory_order_relaxed);
            __asm__ volatile ("addq  $88, %%rsp":"+m"(__not_exist_global_sym_for_asm_seq)::"cc");
            // 还原临时增加的虚拟线程数
            // 这里不会导致线程数减为0，因为本线程还在运行
            if (current_proc)
                atomic_fetch_sub_explicit(&current_proc->threads_num, 1, memory_order_relaxed);
            return thrd_success;
        }
    }
    void *const last_end = atomic_exchange_explicit(&mutex->wait_end, node, memory_order_relaxed);
    if (last_end == NULL) {
        // 成为了owner
        void *temp = node;
        if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &temp, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed))
            goto label_fast_exit;
        if (is_sti)
            __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
        atomic_store_explicit(&mutex->owner, current_thread, memory_order_relaxed);
        __asm__ volatile ("addq  $88, %%rsp":"+m"(__not_exist_global_sym_for_asm_seq)::"cc");
        // 还原临时增加的虚拟线程数
        // 这里不会导致线程数减为0，因为本线程还在运行
        if (current_proc)
            atomic_fetch_sub_explicit(&current_proc->threads_num, 1, memory_order_relaxed);
        while ((temp = *(void *volatile*)node) == NULL)
            asm("pause");
        *(void **)&mutex->waiters = temp;
        return thrd_success;
    } else
        *(void **)last_end = node;



    // 从这里开始栈不可用

    __asm__ volatile goto (
            "rdgsbase   %%rsp\n\t"
            "addq       %0, %%rsp\n\t"
            "jmp        switch_to_empty"
            :
            :"i"(offsetof(struct Core_Data, stack) + sizeof(kernel_gs_base.stack) - 16), "D"(current_proc == NULL ? (struct Proc *)(*(volatile uintptr_t *)&current_thread->proc & -2) : current_proc)
            :"cc"
            :label_wakeup);
    __builtin_unreachable();

    // 线程下次被唤醒就从这里开始运行
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
