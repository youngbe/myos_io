#include <elf.h>

#include <stdnoreturn.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

extern int __virtual_null[];
void kernel_init_part0(void);
struct E820_Entry;
noreturn void kernel_init_part1(const struct E820_Entry* e820_entrys, size_t e820_entrys_num);

// 内核启动协议：
// 1. bootloader 不应配置ap，使用bsp完成内核加载后运行内核（jmp to 内核入口）
//
// 2. bootloader 应该通过cpuid指令确保bsp可以运行system V abi x86-64-v3指令集的所有指令，并且在配置完x86-64-v3运行环境后，才能进入内核，具体包括以下几个方面：
// 2.1 页表：使用PAE分页模式（4级分页）；使用 0 - 512G内存地址 上 f(x) = x 映射关系页表，(使用1G大页进行映射，只需两张页表即可(8K))，保证内核对该区域内存有读写权限；页表保存在 4k - 16M的空闲内存中
// 2.2 段描述符表：使用lgdt加载段描述符表，%cs设为 long mode code descriptor with ring 0, %ds, %es, %fs, %gs, %ss设为0；段描述符表的保存在 4k - 16M 空闲内存中
// 2.3 运行内核前，以System V ABI x86-64-v3 运行新进程的标准正确设置寄存器状态，参考 System V ABI 3.4.1 Register State的部分
// 2.4 %rsp对齐16字节后跳转入内核入口(jmp)
// 2.5 其他System V ABI要求的新进程初始化项目可忽略，如输入参数，环境变量, thread state, Auxiliary Vector 等
// 2.6 配置控制寄存器来开启x86-64-v3，参考下文 控制寄存器说明
//
// 3. 除了保证x86-64-v3外，bootloader还需使用CPUID指令保证bsp具备APIC, x2APIC, FSGSBASE, MSR寄存器(rdmsr/wrmsr指令可用)的功能，同时保证bsp 的 EAX Maximum Input Value for Basic CPUID Information(return to %eax from calling cpuid with %eax == 0) 至少为 0xd；配置cr4寄存器保证FSGSBASE可用（参考下文 控制寄存器说明）
//
// 4. 内核被加载到 16M - 4G 空闲内存上，首尾对齐2M (如内核大小为 2M + 1 字节，选择加载在16M的位置上，则需要保证 16M - 20M 这 4M 都是空闲可用的内存)
//
// 5. 传入BIOS e820中断获取的内存分布，以e820条目数组的形式进行传入，需要对条目进行整理：处理重叠、越界、并排序；e820条目数组保存在 4k - 16M 空闲内存上
// 
// 6. 进入内核时准备至少64K的栈，栈的位置在 4k - 16M 空闲内存上；进入内核时%rsp首先对齐16字节，然后 jmp 内核入口地址
// 
// 7. 进入内核时关闭中断(cli) 和调试(clear TF flag)
//
// 控制寄存器说明：
// 进入内核前，请按照下面所示正确配置控制寄存器：
// cr0[0].PE: 1 to enable x86_64
// cr0[1].MP : 1 to enable x87 && mmx (for x86-64-v3)
// CR0[2].EM: 0 to enable x87 && mmx (for x86-64-v3)
// CR0[3].TS: 0 to enable x87 && mmx (for x86-64-v3)
// cr0[16].WP: 保证内核对 0 - 512G 内存可读/写/执行即可
// cr0[31].PG: 1 to enable x86_64
//
// cr4[5].PAE: 1 to enable x86_64 PAE分页
// cr4[9].OSFXSR: 1 to enable sse (for x86-64-v3)
// CR4[12].LA57: 0 to 禁用5级分页
// cr4[18].OSXSAVE: 1 to 启用xsave (for x86-64-v3)
// cr4[16].FSGSBASE: 1 to 启用FSGSBASE
//
// EFER[0].SCE : 1 to 启用syscall (for x86-64-v3)
// EFER[8].LME : 1 to 启用x86_64
//
// xcr0[0]: 1 to enable x87 (for x86-64-v3)
// xcr0[1]: 1 to enable sse (for x86-64-v3)
// xcr0[2]: 1 to enable avx (for x86-64-v3)
//
// 对于其他上面没有列出的控制寄存器位，内核要么不关心，要么会在内核运行后再设置。对于这些位，建议bootloader不要修改它们（保持BIOS启动后预设值）
__attribute__((section(".text.entry_point")))
noreturn void _start(const struct E820_Entry *const e820_entrys, const size_t e820_entrys_num, void *const free_4kpage)
{
    // 重定位
    {
        extern const Elf64_Rela __rela_dyn_start[];
        extern const Elf64_Rela __rela_dyn_end[];
        const Elf64_Rela* rela = __rela_dyn_start;

        while (rela != __rela_dyn_end) {
            assert(rela->r_info == R_X86_64_RELATIVE);
            *(void **)((uintptr_t)__virtual_null + rela->r_offset) = (void *)((uintptr_t)__virtual_null + rela->r_addend);
            ++rela;
        }
        __asm__ volatile ("":::"memory");
    }

    // 接下来，开始内核的初始化
    //
    // 存在函数：
    // noreturn void kernel_init_part0()
    // noreturn void kernel_init_part1()
    // ...
    //
    // 初始化流程：
    // 调用kernel_init_part0，在函数kernel_init_part0的结尾调用kernel_init_part1，以此类推
    // kernel_init_part0 -> kernel_init_part1 -> kernel_init_part2 -> ...
    //
    // 全部完成后运行 .preinit_array 和 .init_array 的函数
    kernel_init_part0();
    kernel_init_part1(e820_entrys, e820_entrys_num);
}
