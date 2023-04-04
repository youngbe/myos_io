#include "sched-internal.h"

#include "io.h"
#include "driver.h"
#include "mcs_spin.h"

#include <myos_sched.h>

#include <cpuid.h>

#include <stdnoreturn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const mi_heap_t _mi_heap_empty;
extern alignas(4096) const uint64_t kernel_pt1[512];
static struct Proc main_proc = {
    // init by init4
    //.threads_num = 1,

    // main proc will never be killed
    // so pt1 should not be reference
    .pt1 = NULL,
    .vpt1 = NULL,
};
struct Thread main_thread = {
    .__errno = 0,
    ._mi_heap_default = (mi_heap_t*)&_mi_heap_empty,
    .proc = &main_proc,
    .cr3 = kernel_pt1,
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

struct Thread *schedulable_threads = NULL;
spin_mtx_t schedulable_threads_lock = SPIN_MUTEX_INIT_VAL;
volatile _Atomic(ssize_t) schedulable_threads_num = 0;
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


static atomic_size_t cores_num = 1;
static volatile _Atomic(uint32_t) max_xsave_size;

// init bsp's gdt, idt, tss;
// lgdt, wrgsbase, ltr, lidt, config_x2APIC, init max_xsave_size
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
                "wrgsbaseq %[kernel_gs_base]\n\t"
                "ltrw   %[tss]"
                // we change %rsp so use pointer to gdtr
                :[temp]"=&r"(temp)
                :[p_gdt]"r"(&gdtr), [zero]"r"((uint64_t)0), [cs]"i"((uint64_t)8), [rflags]"i"((uint64_t)2), [kernel_gs_base]"r"(&main_core_data), [tss]"r"((uint16_t)40), "m"(gdtr)
                :
                );
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

    // Update xsave size
    {
        uint64_t rax, rbx, rcx, rdx;
        __asm__ volatile(
                "cpuid"
                :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                :"a"((uint32_t)0xd), "c"((uint32_t)0)
                :);
        if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
            __builtin_unreachable();
        atomic_store_explicit(&max_xsave_size, (uint32_t)rbx, memory_order_relaxed);
    }
}

// init ap cores
// lgdt, wrgsbase, ltr, config x2APIC, update max_xsave_size
noreturn void ap_init0(struct Core_Data *const this_core_data)
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
                "wrgsbaseq %[kernel_gs_base]\n\t"
                "ltrw   %[tss]"
                :[temp]"=&r"(temp)
                // we change %rsp so use pointer to gdtr
                :[p_gdt]"r"(&gdtr), [zero]"r"((uint64_t)0), [cs]"i"((uint64_t)8), [rflags]"i"((uint64_t)2), [kernel_gs_base]"r"(this_core_data), [tss]"r"((uint16_t)40), "m"(gdtr)
                :
                );
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
    // Update xsave size
    {
        uint64_t rax, rbx, rcx, rdx;
        __asm__ volatile(
                "cpuid"
                :"=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx)
                :"a"((uint32_t)0xd), "c"((uint32_t)0)
                :);
        if (rax > 0xffffffff || rbx > 0xffffffff || rcx > 0xffffffff || rdx > 0xffffffff)
            __builtin_unreachable();
        const uint32_t this_xsave_size = rbx;
        uint32_t current_max_xsave_size = atomic_load_explicit(&max_xsave_size, memory_order_relaxed);
        while (this_xsave_size > current_max_xsave_size) {
            if (atomic_compare_exchange_strong_explicit(&max_xsave_size, &current_max_xsave_size, this_xsave_size, memory_order_relaxed, memory_order_relaxed))
                break;
        }
    }

    atomic_signal_fence(memory_order_release);

    __asm__ volatile(
            //"movq   %[core_data], %%rsp\n\t"
            //"addq   %[offset], %%rsp\n\t"
            "leaq   %[offset](%[core_data]), %%rsp\n\t"
            "lock incq  %0\n\t"
            "sti\n\t"
            "jmp    empty_loop"
            :"+m"(cores_num)
            :[core_data]"r"(this_core_data), [offset]"m"(*(void **)(uintptr_t)(offsetof(struct Core_Data, stack) + sizeof(this_core_data->stack)))
            :);
    __builtin_unreachable();
}

static inline size_t get_cores_num()
{
    return 8;
}

// 对ap_start16.s ap_start64.s 提供
_Atomic(struct Core_Data *)__ap_startup_code_core_datas;
_Atomic(struct Core_Data *)__ap_startup_code_core_datas_end;

// 由ap_start16.s ap_start64.s 提供
void ap_start16(void);
void ap_start64(void);
extern const unsigned char ap_start16_end[];
extern uint16_t __ap_startup_code_segment;
extern uint32_t __ap_startup_code_cr3;
extern uint32_t __ap_startup_code_jmp_dest;
extern uint32_t __ap_startup_code_gdt_address;
extern atomic_bool __ap_startup_code_error_flag;
// 多核心初始化，完成后修改timer_isr的xsave_area_size
void kernel_init_part4(void *const low_free_4kpage)
{
    const size_t expect_cores_num = get_cores_num();
    main_proc.threads_num = expect_cores_num;
    if (expect_cores_num <= 1) {
label0:
        ;
        extern uint32_t xsave_area_size;
        xsave_area_size = atomic_load_explicit(&max_xsave_size, memory_order_relaxed);
        // 先写入xsave_area_size再修改时钟中断入口
        // 更新时钟中断
        extern void timer_isr(void);
	    atomic_store_explicit((_Atomic(__typeof__(idts[32])) *)&idts[32], IDT_DESCRIPTOR((uintptr_t)&timer_isr, 8), memory_order_release);
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
        ap_core_datas[i].proc = &main_proc;

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
    atomic_store_explicit(&__ap_startup_code_core_datas_end, &ap_core_datas[ap_cores_num], memory_order_relaxed);
    atomic_store_explicit(&idle_cores_num, ap_cores_num, memory_order_relaxed);



    __ap_startup_code_segment = (uintptr_t)low_free_4kpage >> 4;
    __ap_startup_code_cr3 = (uintptr_t)kernel_pt1;
    __ap_startup_code_jmp_dest = (uintptr_t)ap_start64;
    __ap_startup_code_gdt_address += (uintptr_t)low_free_4kpage;

    // 上面几行修改实际上不符合strict-alias (编译器可以认为上面数据不是ap_start16函数的一部分)， 因此加上这么一句话，使得确保写入后再memcpy
    atomic_signal_fence(memory_order_acq_rel);

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

    // 等待十亿次循环
    wait(1'000'000'000);

    // 检查错误 && 处理器数量
    if (atomic_load_explicit((atomic_bool *)((uintptr_t)&__ap_startup_code_error_flag - (uintptr_t)ap_start16 + (uintptr_t)low_free_4kpage), memory_order_seq_cst) || atomic_load_explicit(&cores_num, memory_order_seq_cst) != expect_cores_num) {
        fputs("Error in initing multicores!\n", stderr);
        abort();
    }

    // 现在获取了正确的xsave_size
    goto label0;
}
