    .section .text.ap_start16, "ax", @progbits
    .code16
    .globl ap_start16
    .type ap_start16, @function
    # 目前代码中最大的align为8
    .balign 8
ap_start16:
    cli
    # opcode: ljmpl $__ap_startup_code_segment, $1f - ap_start16
    .byte 0x66, 0xea
    .long 1f - ap_start16
    .globl __ap_startup_code_segment
__ap_startup_code_segment:
    .word 0
1:

    # 配置栈
    movw    %cs, %ax
    movw    %ax, %ds
    movw    %ax, %ss
    movl    $4096, %esp

    # save address of error to %ebp
    movzwl  %ax, %ebp
    shll    $4, %ebp
    addl    $__ap_startup_code_error_flag - ap_start16, %ebp

    # 上锁
    # 我们使用xchgl指令因为不是所有的CPU都支持cmpxchg
    movl    $1, %eax
1:
    xchgl   %eax, .Lspin_mutex - ap_start16
    testl   %eax, %eax
    jne     1b
    # 检查cpuid指令是否可用
    pushfl
    movl    %ss:(%esp), %eax
    xorl    $0x00200000, %ss:(%esp)
    popfl
    pushfl
    popl    %ecx
    # old == %eax, new == %ecx
    xorl    %eax, %ecx
    # %ecx == whichever bits were changed
    testl   $0x00200000, %ecx
    jne     1f
    # 第21位未修改，不支持cpuid
    # 开锁
    movl    $0, .Lspin_mutex - ap_start16
    jmp     .Lcpu_failed
1:
    # restore old eflags
    pushl   %eax
    popfl
    # 开锁
    movl    $0, .Lspin_mutex - ap_start16

    # 检查cpuid最高可调用参数
    # 低 cpuid >= 0dh
    xorl    %eax, %eax
    cpuid
    cmpl    $0xd, %eax
    jb  .Lcpu_failed
    # 高 cpuid >= 0x80000001
    movl    $0x80000000, %eax
    cpuid
    cmpl    $0x80000001, %eax
    jb  .Lcpu_failed

    # 检查是否支持以下特性：
    # x86-64-v3
    # apic
    # x2apic
    # msr registers exist, rdmsr/wrmsr can use
    # fsgsbase
    # 1-GByte pages in LME PAE paging
    movl    $1, %eax
    cpuid
    # edx[0]:fpu
    # edx[5]:msr
    # edx[6]:pae
    # edx[8]:cx8
    # edx[9]:apic
    # edx[15]:cmov
    # edx[23]:mmx
    # edx[24]:fxsr
    # edx[25]:sse
    # edx[26]:sse2
    movl    $(1 << 0) | (1 << 5) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 15) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26), %esi
    andl    %esi, %edx
    cmpl    %esi, %edx
    jne     .Lcpu_failed
    # ecx[0]:sse3
    # ecx[9]:ssse3
    # ecx[12]:fma
    # ecx[13]:cx16
    # ecx[19]:sse4.1
    # ecx[20]:sse4.2
    # ecx[21]:x2apic
    # ecx[22]:movbe
    # ecx[23]:popcnt
    # ecx[26]:xsave
    # ecx[28]:avx
    # ecx[29]:f16c
    movl    $(1 << 0) | (1 << 9) | (1 << 12) | (1 << 13) | (1 << 19)  | (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 26) | (1 << 28) | (1 << 29), %esi
    andl    %esi, %ecx
    cmpl    %esi, %ecx
    jne     .Lcpu_failed

    movl    $0x80000001, %eax
    cpuid
    # ecx[0]: LAHF/SAHF available in 64-bit mode, note LAHF and SAHF are always available in other modes, regardless of the enumeration of this feature flag
    # ecx[5]: lzcnt
    movl    $(1 << 0) | (1 << 5), %esi
    andl    %esi, %ecx
    cmpl    %esi, %ecx
    jne      .Lcpu_failed
    # edx[26]: pdpe1gb : 1-GByte pages in LME PAE paging
    # edx[29]: lm
    movl    $(1 << 26) | (1 << 29), %esi
    andl    %esi, %edx
    cmpl    %esi, %edx
    jne     .Lcpu_failed

    movl    $7, %eax
    xorl    %ecx, %ecx
    cpuid
    # ebx[0]:FSGSBASE
    # ebx[3]:bmi1
    # ebx[5]:avx2
    # ebx[8]:bmi2
    movl    $(1 << 0) | (1 << 3) | (1 << 5) | (1 << 8), %esi
    andl    %esi, %ebx
    cmpl    %esi, %ebx
    jne     .Lcpu_failed

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
    movl    $(1 << 5) | (1 << 9) | (1 << 16) | (1 << 18), %eax
    movl    %eax, %cr4

    # 重新检查CPUID OSXSAVE
    movl    $1, %eax
    cpuid
    # ecx[27].OSXSAVE
    testl   $1 << 27, %ecx
    je      .Lcpu_failed

    # CPUID with %eax == 0xd, %ecx == 0, 返回xcr0可设置位
    movl    $0xd, %eax
    xorl    %ecx, %ecx
    cpuid
    movl    %eax, %esi
    # xcr0[0]: x87
    # xcr0[1]: sse
    # xcr0[2]: avx
    movl    $0b111, %edi
    andl    %edi, %esi
    cmpl    %edi, %esi
    jne     .Lcpu_failed

    # 启用x87, sse, avx, avx2, 如果可能，启用avx-512
    # 禁用MPX, AMX和其他没用的功能
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    andl    $0b11100111, %eax
    xsetbv

    lgdtl   .Lgdt - ap_start16
    
    # opcode: movl  __ap_startup_code_cr3, %eax
    .byte 0x66, 0xb8
    .globl __ap_startup_code_cr3
__ap_startup_code_cr3:
    .long 0
    movl    %eax, %cr3

    # 设置 EFER 寄存器的LME位
    # IA32_EFER.LME[8] : 开启长模式
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $1 << 8, %eax
    wrmsr

    # cr0[0].PE : 1 to enable x86_64
    # cr0[1].MP : 1 to enable x87 && mmx
    # CR0[2].EM: 0 to enable x87 && mmx
    # CR0[3].TS: 0 to enable x87 && mmx
    # CR0[4].ET: 1, reserve
    # cr0[5].NE: clear to disable Numeric error
    # cr0[16].WP: clear to disable write protect
    # cr0[29].NW, cr0[30].CD: clear to enable write-back cache
    # cr0[31].PG: set to enable x86_64
    movl    $(1 << 0) | (1 << 1) | (1 << 4) | (1 << 31), %eax
    movl    %eax, %cr0
    
    # opcode: ljmpl $8, __ap_startup_code_jmp_dest
    .byte 0x66, 0xea
    .globl __ap_startup_code_jmp_dest
__ap_startup_code_jmp_dest:
    .long 0
    .word 0x8



    .balign 8
.Lgdt:
    .word 15
    .globl __ap_startup_code_gdt_address
__ap_startup_code_gdt_address:
    .long .Lgdt - ap_start16
    .word 0

    .long 0
    .byte 0
    .byte 0b10011010
    .byte 0b00100000
    .byte 0

    .balign 4
.Lspin_mutex:
    .long 0


    .globl __ap_startup_code_error_flag
__ap_startup_code_error_flag:
    .byte 0
    
.Lcpu_failed:
    movb    $1, %al
    xchgb   %al, __ap_startup_code_error_flag - ap_start16
1:
    hlt
    jmp 1b

    .globl ap_start16_end
ap_start16_end:

    .size ap_start16, . - ap_start16
