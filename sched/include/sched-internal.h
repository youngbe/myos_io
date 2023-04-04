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
    uint64_t *pt1; // cr3
    void **vpt1;
    struct Thread *proc_threads;
};

struct Thread
{
    alignas(16) struct Thread* next;
    struct Thread *prev;
    void *rsp;
    void *return_hook;
    // 此处cr3只用于线程切换所以我们把他设置为const
    // 如果需要修改页表，请找proc->pt1
    const uint64_t *cr3;
    bool is_killed;
    struct Proc *proc;
    int __errno;
    mi_heap_t* _mi_heap_default;
    uint8_t stack[0x200000 - 72];
};
// make sure can use malloc
static_assert(alignof(struct Thread) == 16);
static_assert(offsetof(struct Thread, is_killed) == 40);
static_assert(sizeof(struct Thread) == 0x200000);

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

static struct Proc main_proc;
extern struct Thread main_thread;
