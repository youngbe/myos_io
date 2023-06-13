    .set BOOTLOADER_STACK_SEGMENT, 0x7000

    .set CSS_64, 8

    .code32
    .section .text.enter64,"ax",@progbits
    .globl enter64
enter64:
    # set cr3
    movl    $pt1, %ebx
    movl    %ebx, %cr3

    # set %cr4.PAE[5] ，设置分页模式为64位分页模式
    movl    %cr4, %eax
    orl     $1 << 5, %eax
    movl    %eax, %cr4

    # 设置 EFER 寄存器的LME位
    # IA32_EFER.LME[8] : 开启长模式
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $1 << 8, %eax
    wrmsr

    # 设置 %cr0 的 PG 位
    movl    %cr0, %ecx
    orl     $1 << 31, %ecx
    movl    %ecx, %cr0

    ljmpl   $CSS_64, $1f
    .code64
1:
    xorl    %eax, %eax
    movq    %rax, %ds
    movq    %rax, %es
    movq    %rax, %fs
    movq    %rax, %gs
    movq    %rax, %ss

    movl    $0x80000001, %eax
    cpuid
    testl   $1 << 11, %edx
    je      .Lnot_support_syscall

    # 设置 EFER 寄存器的LME, SCE位
    movl    $0xC0000080, %ecx
    rdmsr
    # IA32_EFER.SCE[0] : 开启syscall
    orl     $1, %eax
    wrmsr

    # System V ABI 初始化寄存器状态 for init process
    #opcode: ldmxcsr     .Lmxcsr_standard
    .long 0x2514ae0f
    .long .Lmxcsr_standard
    vzeroall
    fninit

    movl    4(%esp), %eax
    movl    8(%esp), %edi
    movl    12(%esp), %esi
    movl    16(%esp), %edx
    movl    20(%esp), %ecx
    jmpq    *%rax

    # 待完善
.Lnot_support_syscall:
    jmp .

    .p2align 2
.Lmxcsr_standard:
    .long 0b111111 << 7
