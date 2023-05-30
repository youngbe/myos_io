#pragma once

#include "myos_sched.h"

#include <al_list.h>

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
    struct FD *stdin_fp;
    struct FD *stdout_fp;
    struct FD *stderr_fp;
};

struct Thread
{
    alignas(16) al_node_t al_node;
    void* temp0;
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
    uintptr_t proc;
    bool is_killed;
    int __errno;
    mi_heap_t* _mi_heap_default;
    uint8_t stack[0x200000 - 64];
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
extern al_index_t schedulable_threads;
extern spin_mtx_t schedulable_threads_lock;
// 在 schedulable_threads 中的线程数量
extern volatile atomic_size_t schedulable_threads_num;
// 不是刚运行换下来的被调度的线程数量
extern volatile _Atomic(ssize_t) old_schedulable_threads_num;
extern volatile atomic_size_t idle_cores_num;

extern struct Thread main_thread;

void __attribute__((noinline)) set_thread_schedulable(struct Thread *new_thread);
void __attribute__((noinline)) cli_set_thread_schedulable(struct Thread *new_thread);
/*
void __attribute__((noinline)) set_threads_schedulable(struct Thread *new_threads, size_t num, uint32_t is_sti, struct Spin_Mutex_Member *p_spin_mutex_member);

__attribute__((noinline, no_caller_saved_registers)) void
set_thread_schedulable_interrupt(struct Thread *new_thread, struct Spin_Mutex_Member *p_spin_mutex_member);
*/

static inline thrd_t thrd_current_inline(void)
{
    thrd_t current_thread;
    __asm__ volatile(
            "movq   %%gs:0, %0"
            :"=r"(current_thread), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    return current_thread;
}

static inline bool check_sti(void)
{
    uint64_t rflags;
    __asm__ volatile (
            "pushfq\n\t"
            "popq   %0"
            :"=r"(rflags), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    return rflags & 512;
}
