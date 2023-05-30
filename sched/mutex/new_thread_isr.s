    .section .text.new_thread_isr, "ax", @progbits
    .globl new_thread_isr
    .type new_thread_isr, @function
    .p2align 4, 0x90
new_thread_isr:
    testl   $0b11, 8(%rsp)
    # 来自用户态，必然已经有线程在运行
    jne     1f
    cmpq    $0, %gs:0
    je      empty_switch_to_empty_interrupt
1:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    xorl    %eax, %eax
    xorl    %edx, %edx
    movl    $0x80b, %ecx
    wrmsr
    popq    %rdx
    popq    %rcx
    popq    %rax
    iretq
    .size new_thread_isr, . - new_thread_isr
