#include "sched-internal.h"

#include "io.h"
#include "driver.h"
#include <spinlock.h>

#include <myos_sched.h>

#include <cpuid.h>

#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct Proc default_proc;

extern const mi_heap_t _mi_heap_empty;
extern alignas(4096) const uint64_t kernel_pt1[512];
struct Thread main_thread = {
    .__errno = 0,
    ._mi_heap_default = (mi_heap_t*)&_mi_heap_empty,
    .proc = (uintptr_t)&default_proc + 1,
    .cr3 = NULL,
    .is_killed = false,
};
static struct Core_Data main_core_data = {
    .running_thread = &main_thread,

    .gdt.null = 0,
    .gdt.code = DESCRIPTOR(0, 0, (0b0010 << 8) | (0b1001 << 4) | 0b1011),
    .gdt.data = DESCRIPTOR(0, 0, (0b0000 << 8) | (0b1001 << 4) | 0b0011),
    .gdt.code_user = DESCRIPTOR(0, 0, (0b0010 << 8) | (0b1111 << 4) | 0b1011),
    .gdt.data_user = DESCRIPTOR(0, 0, (0b0000 << 8) | (0b1111 << 4) | 0b0011),
    //.gdt.tss_low = DESCRIPTOR((uintptr_t)&main_core_data.tss, 103, (0b0000 << 8) | (0b1000 << 4) | 0b1001),
    //.gdt.tss_high = (uintptr_t)&main_core_data.tss >> 32,

    .tss.rsp0 = (uintptr_t)&main_thread.stack[sizeof(main_thread.stack) / sizeof(main_thread.stack[0])],
};

al_index_t schedulable_threads = AL_INDEX_INIT_VAL;
spin_mtx_t schedulable_threads_lock = SPIN_MUTEX_INIT_VAL;
volatile atomic_size_t schedulable_threads_num = 0;
volatile _Atomic(ssize_t) old_schedulable_threads_num = 0;
// 由kernel_init_part4进行初始化
volatile atomic_size_t idle_cores_num;

#define IDT_DESCRIPTOR(entry_point, segment_selector) \
    (((__uint128_t)(entry_point) & 0xffff) | \
    (((__uint128_t)(entry_point) & 0xffff0000) << 32) | \
    (((__uint128_t)(entry_point) & 0xffffffff00000000) << 64) | \
    (((__uint128_t)(segment_selector) & 0xffff) << 16) | \
    ((__uint128_t)0b1000 << 44) | ((__uint128_t)0b1110 << 40) | ((__uint128_t)0b0000 << 36) | ((__uint128_t)0b0000 << 32))
static __uint128_t idts[256];


// init idt
// init bsp's gdt, idtr, tss, gsbase, x2apic;
void kernel_init_part1(void)
{
    main_core_data.gdt.tss_low = DESCRIPTOR((uintptr_t)&main_core_data.tss, 103, (0b0000 << 8) | (0b1000 << 4) | 0b1001);
    main_core_data.gdt.tss_high = (uintptr_t)&main_core_data.tss >> 32;

    for (size_t i = 0; i < 255; ++i)
        idts[i] = IDT_DESCRIPTOR((uintptr_t)&abort, 8);
    {
        // 时钟中断
        extern void empty_isr(void);
        idts[32] = IDT_DESCRIPTOR((uintptr_t)&empty_isr, 8);
    }
    {
        // 键盘中断
        extern void keyboard_isr(void *);
        idts[33] = IDT_DESCRIPTOR((uintptr_t)&keyboard_isr, 8);
    }
    {
        // kernel_abort中断
        extern void abort_handler(void);
        idts[34] = IDT_DESCRIPTOR((uintptr_t)&abort_handler, 8);
    }
    {
        // 新线程中断
        extern void new_thread_isr(void);
        idts[35] = IDT_DESCRIPTOR((uintptr_t)&new_thread_isr, 8);
    }
    {
        // 虚假中断
        extern void spurious_isr(void);
        idts[255] = IDT_DESCRIPTOR((uintptr_t)&spurious_isr, 8);
    }

    atomic_signal_fence(memory_order_acq_rel);
    // lgdt then wrgsbase && ltr tss
    {
        alignas(16) struct __attribute__((packed)) {
            uint16_t limit;
            uint64_t base;
        } gdtr = {7 * 8 - 1, (uintptr_t)&main_core_data.gdt};
        uint64_t temp;
        __asm__ volatile(
                "movq   %%rsp, %[temp]\n\t"
                "pushq  %[zero]\n\t"
                "pushq  %[temp]\n\t"
                "leaq   1f(%%rip), %[temp]\n\t"
                "pushq  %[rflags]\n\t"
                "pushq  %[cs]\n\t"
                "pushq  %[temp]\n\t"
                "lgdtq (%[p_gdt])\n\t"
                "iretq\n"
                "1:\n\t"
                "movq   %[zero], %%ds\n\t"
                "movq   %[zero], %%es\n\t"
                "movq   %[zero], %%fs\n\t"
                "movq   %[zero], %%gs\n\t"
                "wrgsbase %[kernel_gs_base]\n\t"
                "ltrw   %[tss]"
                // we change %rsp so use pointer to gdtr
                :[temp]"=&r"(temp)
                :[p_gdt]"r"(&gdtr), [zero]"r"((uint64_t)0), [cs]"i"((uint64_t)8), [rflags]"i"((uint64_t)2), [kernel_gs_base]"r"(&main_core_data), [tss]"r"((uint16_t)40), "m"(gdtr)
                :"cc");
    }
    // lidt
    {
        alignas(16) struct __attribute__((packed)) {
            uint16_t limit;
            uint64_t base;
        } idtr = {sizeof(idts) - 1, (uintptr_t)&idts};
        __asm__ volatile(
                "lidtq %0"
                :
                :"m"(idtr)
                :);
    }

    // 现在可以用: thrd_current
    // mtx_lock/mtx_unlock (依赖于thrd_current)
    // printf (依赖于mtx_lock)
    puts("kernel can use printf now!");


    puts("config x2apic ...");
    config_x2apic();
}

// 对ap_start16.s ap_start64.s 提供

// 所有核心数量
size_t __ap_startup_code_all_cores_num;
volatile _Atomic(uint32_t) __ap_startup_code_xcr0_usable_bits = 0b11100111;
// 已经填写了xcr0_usable_bits的核心数量
// 当这个数量满了的时候，可以填写max_xsave_size
volatile atomic_size_t __ap_startup_code_written_xcr0_usable_bits_cores_num = 1;
volatile _Atomic(struct Core_Data *) __ap_startup_code_core_datas;
static volatile _Atomic(uint32_t) __ap_startup_code_max_xsave_size = 0;
// 已经完成了所有初始化的核心数量
static volatile atomic_size_t __ap_startup_code_init_finished_cores_num = 0;

// 由ap_start16.s ap_start64.s 提供
void ap_start16(void);
void ap_start64(void);
extern const unsigned char ap_start16_end[];
extern uint16_t __ap_startup_code_segment;
extern uint32_t __ap_startup_code_cr3;
extern uint32_t __ap_startup_code_jmp_dest;
extern uint32_t __ap_startup_code_gdt_address;
extern atomic_bool __ap_startup_code_error_flag;

// init ap cores
// lgdt, wrgsbase, ltr, config x2APIC, update max_xsave_size
noreturn void ap_init0(struct Core_Data *const this_core_data, const size_t all_cores_num)
{
    // lgdt, and then wrgsbase && ltr
    {
        alignas(16) struct __attribute__((packed)) {
            uint16_t limit;
            uint64_t base;
        } gdtr = {7 * 8 - 1, (uintptr_t)&this_core_data->gdt};
        uint64_t temp;
        __asm__ volatile(
                "movq   %%rsp, %[temp]\n\t"
                "pushq  %[zero]\n\t"
                "pushq  %[temp]\n\t"
                "leaq   1f(%%rip), %[temp]\n\t"
                "pushq  %[rflags]\n\t"
                "pushq  %[cs]\n\t"
                "pushq  %[temp]\n\t"
                "lgdtq (%[p_gdt])\n\t"
                "iretq\n"
                "1:\n\t"
                "movq   %[zero], %%ds\n\t"
                "movq   %[zero], %%es\n\t"
                "movq   %[zero], %%fs\n\t"
                "movq   %[zero], %%gs\n\t"
                "wrgsbase %[kernel_gs_base]\n\t"
                "ltrw   %[tss]"
                :[temp]"=&r"(temp)
                // we change %rsp so use pointer to gdtr
                :[p_gdt]"r"(&gdtr), [zero]"r"((uint64_t)0), [cs]"i"((uint64_t)8), [rflags]"i"((uint64_t)2), [kernel_gs_base]"r"(this_core_data), [tss]"r"((uint16_t)40), "m"(gdtr)
                :"cc");
    }
    // lidt
    {
        alignas(16) struct __attribute__((packed)) {
            uint16_t limit;
            uint64_t base;
        } idtr = {sizeof(idts) - 1, (uintptr_t)&idts};
        __asm__ volatile(
                "lidtq %0"
                :
                :"m"(idtr)
                :);
    }
    // 配置x2APIC
    config_x2apic();

    // enable avx-512 if possible
    while (atomic_load_explicit(&__ap_startup_code_written_xcr0_usable_bits_cores_num, memory_order_acquire) != all_cores_num) {}
    uint32_t xcr0_usable_bits = atomic_load_explicit(&__ap_startup_code_xcr0_usable_bits, memory_order_relaxed);
    __asm__ volatile (
            "xsetbv"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"c"((uint32_t)0), "a"(xcr0_usable_bits), "d"((uint32_t)0)
            :);
    // Update max_xsave_size
    uint64_t rax, rbx, rcx, rdx;
    __asm__ volatile (
            "cpuid"
            :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx), "+m"(__not_exist_global_sym_for_asm_seq)
            :"a"((uint32_t)0xd), "c"((uint32_t)0)
            :);
    if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
        __builtin_unreachable();
    const uint32_t this_xsave_size = rbx;
    uint32_t current_max_xsave_size = atomic_load_explicit(&__ap_startup_code_max_xsave_size, memory_order_relaxed);
    while (this_xsave_size > current_max_xsave_size) {
        if (atomic_compare_exchange_strong_explicit(&__ap_startup_code_max_xsave_size, &current_max_xsave_size, this_xsave_size, memory_order_relaxed, memory_order_relaxed))
            break;
    }

    atomic_signal_fence(memory_order_release);

    static_assert(offsetof(struct Core_Data, stack) + sizeof(this_core_data->stack) == 65536);
    __asm__ volatile(
            "leaq   65536(%[core_data]), %%rsp\n\t"
            "lock incq  %0\n\t"
            "sti\n\t"
            "jmp    empty_loop"
            :"+m"(__ap_startup_code_init_finished_cores_num)
            :[core_data]"r"(this_core_data)
            :"cc");
    __builtin_unreachable();
}

static inline size_t get_cores_num()
{
    return 8;
}

// enable avx-512 if possible
// 多核心初始化
// set xsave_size in timer_isr
// set idle_cores_num
void kernel_init_part4(void *const low_free_4kpage)
{
    const size_t expect_cores_num = get_cores_num();
    uint32_t bsp_xcr0_usable_bits;
    {
        uint64_t rax, rbx, rcx, rdx;
        __asm__ volatile (
                "cpuid"
                :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                :"a"((uint32_t)0xd), "c"((uint32_t)0)
                :);
        if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
            __builtin_unreachable();
        bsp_xcr0_usable_bits = rax;
    }
    if (expect_cores_num <= 1) {
        // 只有一个核心的情况
        bsp_xcr0_usable_bits &= 0b11100111;
        __asm__ volatile (
                "xsetbv"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :"c"((uint32_t)0), "a"(bsp_xcr0_usable_bits), "d"((uint32_t)0)
                :);
        uint64_t rax, rbx, rcx, rdx;
        __asm__ volatile (
                "cpuid"
                :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx), "+m"(__not_exist_global_sym_for_asm_seq)
                :"a"((uint32_t)0xd), "c"((uint32_t)0)
                :);
        if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
            __builtin_unreachable();
        extern uint32_t xsave_area_size;
        xsave_area_size = rbx;
        // 更新时钟中断
        extern void timer_isr(void);
        idts[32] = IDT_DESCRIPTOR((uintptr_t)&timer_isr, 8);
        // idle_cores_num = 0;
        return;
    }


    const size_t ap_cores_num = expect_cores_num - 1;
    // 为每个核心申请Core_Data
    struct Core_Data *const ap_core_datas = aligned_alloc(32, ap_cores_num * sizeof(struct Core_Data));
    if (ap_core_datas == NULL) {
        fputs("malloc array for threads failed! Maybe too many cores !\n", stderr);
        abort();
    }
    // 初始化每个AP核心数据
    for (size_t i = 0; i < ap_cores_num; ++i) {
        // 运行空线程，挂在主进程上
        ap_core_datas[i].running_thread = NULL;
        ap_core_datas[i].proc = NULL;

        memset(&ap_core_datas[i].tss, 0, sizeof(ap_core_datas[i].tss));
        // 不需要初始化rsp0

        ap_core_datas[i].gdt.null = 0;
        ap_core_datas[i].gdt.code = DESCRIPTOR(0, 0, (0b0010 << 8) | (0b1001 << 4) | 0b1011);
        ap_core_datas[i].gdt.data = DESCRIPTOR(0, 0, (0b0000 << 8) | (0b1001 << 4) | 0b0011);
        ap_core_datas[i].gdt.data_user = DESCRIPTOR(0, 0, (0b0000 << 8) | (0b1111 << 4) | 0b0011);
        ap_core_datas[i].gdt.code_user = DESCRIPTOR(0, 0, (0b0010 << 8) | (0b1111 << 4) | 0b1011);
        ap_core_datas[i].gdt.tss_low = DESCRIPTOR((uintptr_t)&ap_core_datas[i].tss, 103, (0b0000 << 8) | (0b1000 << 4) | 0b1001);
        ap_core_datas[i].gdt.tss_high = (uintptr_t)&ap_core_datas[i].tss >> 32;
    }
    atomic_store_explicit(&__ap_startup_code_core_datas, ap_core_datas, memory_order_relaxed);



    __ap_startup_code_all_cores_num = expect_cores_num;
    atomic_fetch_and_explicit(&__ap_startup_code_xcr0_usable_bits, bsp_xcr0_usable_bits, memory_order_relaxed);
    __ap_startup_code_segment = (uintptr_t)low_free_4kpage >> 4;
    __ap_startup_code_cr3 = (uintptr_t)kernel_pt1;
    __ap_startup_code_jmp_dest = (uintptr_t)ap_start64;
    __ap_startup_code_gdt_address += (uintptr_t)low_free_4kpage;

    // 先写入空闲内核数量再唤醒ap
    atomic_store_explicit(&idle_cores_num, ap_cores_num, memory_order_relaxed);
    
    default_proc.threads_num = expect_cores_num + 1;

    // 上面几行修改实际上不符合strict-alias (编译器可以认为上面数据不是ap_start16函数的一部分)， 因此加上这么一句话，使得确保写入后再memcpy
    atomic_signal_fence(memory_order_release);

    memcpy(low_free_4kpage, ap_start16, (uintptr_t)ap_start16_end - (uintptr_t)ap_start16);
    // 刷新缓存
    // 清空APIC错误
    wrmsr_volatile_seq(0x828, 0);
    // 广播INIT信息
    wrmsr_volatile_seq(0x830, 0b101 << 8 | 1 << 14 | 0b11 << 18);

    // 等待十亿次循环
    wait(1'000'000'000);
    // 广播SIPI信息
    wrmsr_volatile_seq(0x830, (uintptr_t)low_free_4kpage >> 12 | 0b110 << 8 | 1 << 14 | 0b11 << 18);

    while (atomic_load_explicit(&__ap_startup_code_written_xcr0_usable_bits_cores_num, memory_order_acquire) != expect_cores_num) {}
    uint32_t xcr0_usable_bits = atomic_load_explicit(&__ap_startup_code_xcr0_usable_bits, memory_order_relaxed);
    __asm__ volatile (
            "xsetbv"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"c"((uint32_t)0), "a"(xcr0_usable_bits), "d"((uint32_t)0)
            :);
    uint64_t rax, rbx, rcx, rdx;
    __asm__ volatile (
            "cpuid"
            :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx), "+m"(__not_exist_global_sym_for_asm_seq)
            :"a"((uint32_t)0xd), "c"((uint32_t)0)
            :);
    if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
        __builtin_unreachable();
    const uint32_t this_xsave_size = rbx;
    while (atomic_load_explicit(&__ap_startup_code_init_finished_cores_num, memory_order_acquire) != ap_cores_num) {}
    const uint32_t current_max_xsave_size = atomic_load_explicit(&__ap_startup_code_max_xsave_size, memory_order_relaxed);

    extern uint32_t xsave_area_size;
    xsave_area_size = this_xsave_size > current_max_xsave_size ? this_xsave_size : current_max_xsave_size;

    // 等待1G循环
    wait(1'000'000'000);

    // 检查错误
    if (atomic_load_explicit((atomic_bool *)((uintptr_t)&__ap_startup_code_error_flag - (uintptr_t)ap_start16 + (uintptr_t)low_free_4kpage), memory_order_seq_cst)) {
        fputs("Error in initing multicores!\n", stderr);
        abort();
    }

    // 原子地更新时钟中断
    extern void timer_isr(void);
    const __uint128_t temp = IDT_DESCRIPTOR((uintptr_t)&timer_isr, 8);
    __asm__ volatile (
            "movdqa     %1, %%xmm0\n\t"
            "movdqa     %%xmm0, %0"
            :"=m"(idts[32])
            :"m"(temp)
            :"xmm0");
    //atomic_store_explicit((_Atomic(__typeof__(idts[32])) *)&idts[32], IDT_DESCRIPTOR((uintptr_t)&timer_isr, 8), memory_order_release);
}
