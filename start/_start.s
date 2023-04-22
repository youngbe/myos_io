// 内核启动协议：
// 1. bootloader 不应配置ap，使用bsp完成内核加载后运行内核（jmp to 内核入口）
//
// 2. bootloader 应该通过cpuid指令确保bsp可以运行system V abi x86-64-v3指令集的所有指令，并且在配置完x86-64-v3运行环境后，才能进入内核，具体包括以下几个方面：
// 2.1 页表：使用PAE分页模式（4级分页）；使用 0 - 512G内存地址 上 f(x) = x 映射关系页表，(使用1G大页进行映射，只需两张页表即可(8K))，保证内核对该区域内存有读写权限；页表保存在 4k - 16M的空闲内存中
// 2.2 段描述符表：使用lgdt加载段描述符表，%cs设为 long mode code descriptor with ring 0, %ds, %es, %fs, %gs, %ss设为0；段描述符表的保存在 4k - 16M 空闲内存中
// 2.3 运行内核前，以System V ABI x86-64-v3 运行新进程的标准正确设置寄存器状态(%rsp除外)，参考 System V ABI 3.4.1 Register State的部分
// 2.4 其他System V ABI要求的新进程初始化项目可忽略，如输入参数，环境变量, thread state, Auxiliary Vector 等
// 2.5 配置控制寄存器来开启x86-64-v3，参考下文 控制寄存器说明
//
// 3. 除了保证x86-64-v3外，bootloader还需使用CPUID指令保证bsp具备APIC, x2APIC, FSGSBASE, MSR寄存器(rdmsr/wrmsr指令可用)的功能，同时保证bsp 的 EAX Maximum Input Value for Basic CPUID Information(return to %eax from calling cpuid with %eax == 0) 至少为 0xd；配置cr4寄存器保证FSGSBASE可用（参考下文 控制寄存器说明）
//
// 4. 内核被加载到 16M - 4G 空闲内存上，首尾对齐2M (如内核大小为 2M + 1 字节，选择加载在16M的位置上，则需要保证 16M - 20M 这 4M 都是空闲可用的内存)
//
// 5. 传入BIOS e820中断获取的内存分布，以e820条目数组的形式进行传入，需要对条目进行整理：处理重叠、越界、并排序；e820条目数组保存在 4k - 16M 空闲内存上
// 
// 6. 进入内核前禁用8259a芯片
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


    # 将_start函数放入.text.entry_point节，配合ld脚本使内核入口地址为内核文件的开头
    .section    .text.entry_point,"ax",@progbits
    .globl  _start                          # -- Begin function _start
    .type   _start,@function
    # 内核从这里开始运行
_start:
    movq    %rdi, %rbp
    movq    %rsi, %r12
    movq    %rdx, %r13

    # 换栈
    leaq    main_thread+0x200000(%rip), %rsp

    # 重置控制寄存器
    # cr0[0].PE : 1 to enable x86_64
    # cr0[1].MP : 1 to enable x87 && mmx
    # CR0[2].EM: 0 to enable x87 && mmx
    # CR0[3].TS: 0 to enable x87 && mmx
    # CR0[4].ET: 1, reserve
    # cr0[5].NE: clear to disable Numeric error
    # cr0[16].WP: clear to disable write protect
    # cr0[29].NW, cr0[30].CD: clear to enable write-back cache
    # cr0[31].PG: set to enable x86_64
    movq    $(1 << 0) | (1 << 1) | (1 << 4) | (1 << 31), %rax
    movq    %rax, %cr0

    #xorq    %rax, %rax
    #movq    %rax, %cr2

    # cr4[0].VME: 0 to 禁用Virtual 8086 Mode Extensions
    # cr4[1].PVI: 0 to 禁用Protected-mode Virtual Interrupts
    # cr4[2].TSD: 0 to 允许用户态执行rdtsc
    # cr4[3].DE: 0 to 禁用Debugging Extensions
    # cr4[4].PSE: 0, x86_64 ignore
    # cr4[5].PAE: 1 to enable x86_64 PAE分页
    # cr4[6].MCE: 0 to 禁用Machine Check Exception
    # cr4[7].PGE: 1
    # cr4[8].PCE: 0 to 允许用户态执行rdpmc
    # cr4[9].OSFXSR: 1 to 启用FXSAVE and FXRSTOR
    # cr4[10].OSXMMEXCPT: 0, 视浮点异常为#UD
    # cr4[11].UMIP: 0 to 允许用户运行SGDT, SIDT, SLDT, SMSW and STR
    # CR4[12].LA57: 0 to 禁用5级分页
    # cr4[13].VMXE: 0 to 禁用Virtual Machine Extensions
    # cr4[14].SMXE: 0 to 禁用Safer Mode Extensions （与TPM相关）
    # cr4[16].fsgsbase: 1 to 启用fsgsbase
    # cr4[18].OSXSAVE: 1 to 启用xsave
    # cr4[20].SMEP: 0 to 允许内核运行用户代码
    # cr4[21].SMAP: 0 to 允许内核访问用户数据
    # cr4[22].PKE: 0 to 禁用Protection Key
    # cr4[23].CET: 0 to 禁用 CET
    # cr4[24].PKS: 0 to 禁用 Protection Keys for Supervisor-Mode Pages
    movq    $(1 << 5) | (1 << 9) | (1 << 16) | (1 << 18), %rax
    movq    %rax, %cr4

    ldmxcsr __mxcsr_status_fast_math(%rip)

    callq   pie_relocate
    // init tty
    callq   kernel_init_part0
    // init idt, bsp's gdt, idtr, tss, x2apic, gs_base
    callq   kernel_init_part1
    // init mmap
    movq    %rbp, %rdi
    movq    %r12, %rsi
    callq   kernel_init_part2
    // init mimalloc
    callq   kernel_init_part3
    // init multicore:
    // init ap's gdt, idt, tss, gsbase, x2apic
    // update timer_isr.xsave_area_size
    // update idle_cores_num
    // update timer_isr
    movq    %r13, %rdi
    callq   kernel_init_part4
    // init stdio lock
    callq   kernel_init_part5
    // init tty && keyboard_thread
    callq   kernel_init_part6

    sti

    leaq    __preinit_array_start(%rip), %rbp
    leaq    __preinit_array_end(%rip), %rbx
1:
    cmpq    %rbp, %rbx
    je      1f
    callq   *(%rbp)
    addq    $8, %rbp
    jmp     1b
1:

    callq   kernel_main

    .globl __mxcsr_status_fast_math
__mxcsr_status_fast_math:
    # set MXCSR[15].FZ = 1: for fast-math
    # set MXCSR[6].DAZ = 1: for fast-math
    .long (1 << 6) | (0b111111 << 7) | (1 << 15)

	.size	_start, . - _start
