#pragma once

#include "myos_sched.h"

#include <threads.h>
#include <assert.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>

struct Proc
{
    atomic_size_t threads_num;
    // 对于每个进程，他们的页表都不相同
    // if (proc1 != proc2)
    //      assert(proc1->pt1 != proc2->pt1)
    uint64_t *pt1; // cr3
    void **vpt1;
    struct Thread *proc_threads;
    struct TTY *tty;
};

struct Thread
{
    alignas(16) struct Thread* next;
    struct Thread *prev;
    // 线程切换协议：
    // 在cli的情况下，先设置rsp，然后jmp return_hook
    // 沉睡的线程rsp以下128字节的区域可以被临时使用
    void *rsp;
    void *return_hook;
    // 对于内核线程，此值为NULL
    // 对于普通线程，此值为proc->pt1的复制值
    const uint64_t *cr3;
    // 对于内核线程，此值不断变化，有 (proc & 1) == 1 ，(proc & -2) 当前指向虚拟进程
    // 对于普通线程，此值指向所属进程
    struct Proc *proc;
    bool is_killed;
    int __errno;
    mi_heap_t* _mi_heap_default;
    uint64_t temp0;
    uint8_t stack[0x200000 - 72];
};
// make sure can use malloc
static_assert(alignof(struct Thread) == 16);
static_assert(sizeof(struct Thread) == 0x200000);
static_assert(offsetof(struct Thread, stack) + sizeof(((struct Thread *)(uintptr_t)0)->stack) == sizeof(struct Thread));

// percpu data, use kernel_gs_base to find it
struct Core_Data
{
    struct Thread *running_thread;
    // Only when running_thread == NULL, 这个成员才有意义
    struct Proc *proc;

    // TSS
    alignas(32) struct __attribute__ ((packed))
    {
        uint32_t reserved0;
        uint64_t rsp0, rsp1, rsp2;
        uint64_t reserved1;
        uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
        uint64_t reserved2;
        uint16_t reserved3;
        uint16_t io_map_base_address;
    } tss;

#define DESCRIPTOR(base, limit, flag) \
    ((uint64_t)((((uint64_t)(base) & 0xff000000) << 32) | \
        (((uint64_t)(base) & 0xffffff) << 16) | \
        ((uint64_t)(limit) & 0xffff) | \
        (((uint64_t)(limit) & 0xf0000) << 32) | \
        (((uint64_t)(flag) & 0xff) << 40) | \
        (((uint64_t)(flag) & 0xf00) << 44)))
    // GDT
    alignas(32) struct
    {
        uint64_t null;
        uint64_t code;
        uint64_t data;
        uint64_t data_user;
        uint64_t code_user;
        uint64_t tss_low;
        uint64_t tss_high;
    } gdt;

    // empty_stack, pad to 64K
    uint8_t stack[65320];
};
static_assert(sizeof(struct Core_Data) == 0x10000);
// make sure use aligned_alloc(32)
static_assert(alignof(struct Core_Data) == 32);
static_assert(offsetof(struct Core_Data, stack) + sizeof(((struct Core_Data *)(uintptr_t)0)->stack) == sizeof(struct Core_Data));


struct Spin_Mutex_Member;
typedef struct Spin_Mutex_Member *spin_mtx_t;

// extern 这两个变量因为需要被汇编函数使用
extern struct Thread *schedulable_threads;
extern spin_mtx_t schedulable_threads_lock;
// 在 schedulable_threads 中的线程数量
extern volatile _Atomic(ssize_t) schedulable_threads_num;
// 不是刚运行换下来的被调度的线程数量
extern volatile _Atomic(ssize_t) old_schedulable_threads_num;
extern volatile atomic_size_t idle_cores_num;

extern struct Thread main_thread;

void __attribute__((noinline)) set_thread_schedulable(struct Thread *new_thread, uint32_t is_sti, struct Spin_Mutex_Member *p_spin_mutex_member);
void __attribute__((noinline)) set_threads_schedulable(struct Thread *new_threads, size_t num, uint32_t is_sti, struct Spin_Mutex_Member *p_spin_mutex_member);

#include "mcs_spin.h"
#include <io.h>
#include <utlist.h>
static __attribute__((always_inline)) void set_thread_schedulable_inline(struct Thread *const new_thread, const uint32_t is_sti, struct Spin_Mutex_Member *const p_spin_mutex_member)
{
    if (new_thread == NULL)
        __builtin_unreachable();
    if (is_sti)
        __asm__ volatile ("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    atomic_signal_fence(memory_order_acq_rel);
    spin_lock(&schedulable_threads_lock, p_spin_mutex_member);
    {
        atomic_fetch_add_explicit(&schedulable_threads_num, 1, memory_order_acquire);
        struct Thread *fake_schedulable_threads = schedulable_threads;
        DL_APPEND(fake_schedulable_threads, new_thread);
        schedulable_threads = fake_schedulable_threads;
    }
    spin_unlock(&schedulable_threads_lock, p_spin_mutex_member);
    atomic_signal_fence(memory_order_acq_rel);
    if (atomic_load_explicit(&schedulable_threads_num, memory_order_relaxed) > 0 && atomic_load_explicit(&idle_cores_num, memory_order_relaxed) > 0)
        wrmsr_volatile_seq(0x830, 35 | 0b000 << 8 | 1 << 14 | 0b11 << 18);
    atomic_fetch_add_explicit(&old_schedulable_threads_num, 1, memory_order_acq_rel);
    if (is_sti)
        __asm__ volatile ("sti":"+m"(__not_exist_global_sym_for_asm_seq)::);
}
