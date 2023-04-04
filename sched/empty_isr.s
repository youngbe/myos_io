    .section .text.empty_isr, "ax", @progbits
    .globl empty_isr
    .p2align    4, 0x90
    .type   empty_isr,@function
empty_isr:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    movl    $2059, %ecx                     # imm = 0x80B
    xorl    %eax, %eax
    xorl    %edx, %edx
    wrmsr
    popq    %rdx
    popq    %rcx
    popq    %rax
    iretq
    .size empty_isr, . - empty_isr
