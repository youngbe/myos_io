#include <threads.h>

#include "sched-internal.h"

#include <utlist.h>

#include <stdlib.h>
#include <stdatomic.h>


int __attribute__((noinline)) mtx_lockx(mtx_t*const mtx, const size_t core_id_16, const thrd_t current_thread)
{
    if((core_id_16 & 0xf) != 0)
        __builtin_unreachable();

    __typeof__((core_res)) fake_core_res;
    __asm__ volatile(
            "movq   %%r9, %0"
            :"=r"(fake_core_res)
            :
            :"rax", "rcx", "r8");
    if (fake_core_res != core_res)
        __builtin_unreachable();

    // 保存上下文
    uint64_t rflags;
    __asm__ (
            "pushfq\n\t"
            "movq (%%rsp), %0"
            :"=r"(rflags)
            :
            :"rbx", "rbp", "r12", "r13", "r14", "r15",
            "st", "mm0", "mm1", "mm2", "mm3",
                            "mm0","mm1","mm2","mm3","mm4","mm5","mm6","mm7",
                "xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15",
                "ymm0","ymm1","ymm2","ymm3","ymm4","ymm5","ymm6","ymm7","ymm8","ymm9","ymm10","ymm11","ymm12","ymm13","ymm14","ymm15",
                "zmm0","zmm1","zmm2","zmm3","zmm4","zmm5","zmm6","zmm7","zmm8","zmm9","zmm10","zmm11","zmm12","zmm13","zmm14","zmm15"
            );//"cc", "rbx", "rbp", "r12", "r13", "r14", "r15",
              //"st", "mm0",
              //"zmm0", "zmm16");
    const __auto_type _core_res = &fake_core_res[core_id_16>>4];
    if (current_thread != _core_res->running_thread)
        __builtin_unreachable();
    void *const empty_stack_rsp = _core_res->empty_stack_rsp;
    if (rflags & 0x200) {
        __asm__ (
                "cli"
                :
                :
                :);
    }

    __asm__ (
            "movq   %%rsp, %0"
            :"=m"(current_thread->rsp)
            :
            :);
    current_thread->return_hook = &&label_wakeup;
    // 上下文保存完成


    {
        uint64_t unlocked = 0;
        while (!atomic_compare_exchange_weak_explicit(&mtx->spin_mtx, &unlocked, 1, memory_order_relaxed, memory_order_relaxed))
            unlocked = 0;
    }
    // 确保当前owner != NULL
    {
        thrd_t mtx_owner = NULL;
        if (atomic_compare_exchange_strong_explicit(&mtx->owner, &mtx_owner, current_thread, memory_order_relaxed, memory_order_relaxed)) {
            assert(mtx->count <= 1);
            atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
            goto label_return;
        }
    }
    CDL_APPEND(mtx->blocked_threads, current_thread);

    {
        uint64_t unlocked = 0;
        while (!atomic_compare_exchange_weak_explicit(&schedulable_threads_lock, &unlocked, 1, memory_order_relaxed, memory_order_relaxed))
            unlocked = 0;
    }
    struct Thread *fake_schedulable_threads = schedulable_threads;
    if (fake_schedulable_threads == NULL) {
        atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_relaxed);
        extern alignas(4096) const uint64_t kernel_pt0[512];
        __asm__ (
                "movq   %0, %%cr3"
                :
                :"r"(&kernel_pt0)
                :);
        atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
        __asm__ goto(
                "movq   %0, %%rsp\n\t"
                "sti\n\t"
                "jmp   empty_loop"
                :
                :"r"(empty_stack_rsp)
                :
                :label_wakeup);
    } else {
        struct Thread *const new_thread = fake_schedulable_threads;
        CDL_DELETE2(fake_schedulable_threads, new_thread, schedulable_threads_prev, schedulable_threads_next);
        schedulable_threads = fake_schedulable_threads;
        atomic_store_explicit(&schedulable_threads_lock, 0, memory_order_relaxed);
        {
            uint64_t *current_cr3;
            __asm__ (
                    "movq   %%cr3, %0"
                    :"=r"(current_cr3)
                    :
                    :);
            if (current_cr3 != new_thread->cr3) {
                __asm__ (
                        "movq   %0, %%cr3"
                        :
                        :"r"(new_thread->cr3)
                        :);
            }
        }
        atomic_store_explicit(&mtx->spin_mtx, 0, memory_order_relaxed);
        _core_res->running_thread = new_thread;

        // 切换至新的上下文
        __asm__ goto(
                "movq   %0, %%rsp\n\t"
                "jmpq   *%1"
                :
                :"m"(new_thread->rsp), "m"(new_thread->return_hook)
                :
                :label_wakeup);
    }

    __builtin_unreachable();


label_return:
    if (rflags & 0x200) {
        __asm__ (
                "sti"
                :
                :
                :);
    }
    __asm__ (
            "addq   $8, %%rsp"
            :
            :
            :);
    return thrd_success;
    // 线程下次被唤醒就从这里开始运行
label_wakeup:
    __asm__ (
            "popfq"
            :
            :
            :);
    return thrd_success;
}
