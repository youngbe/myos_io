    .section .text.thread_start, "ax", @progbits
    .globl thread_start
    .p2align    4, 0x90
    .type   thread_start,@function
thread_start:
    # 参考 system V abi 3.4 Process Initialization
    # 设置 rflags,先把中断开起来
    pushq   $0x202
    popfq
    # 设置mxcsr，参考 intel sdm vol 1
    # MXCSR[6].DAZ : 0
    # MXCSR[7-12] : 屏蔽浮点异常
    # MXCSR[13-14].RC ： 控制舍入方式
    pushq   $(0b111111 << 7)
    ldmxcsr (%rsp)
    # 设置 x87 control word, 参考 intel sdm vol-1,8.1.5 x87 FPU Control Word
    movl    $(0b111111 | (0b11 << 8)), (%rsp)
    fldcw   (%rsp)
    popq    %rcx

    vzeroupper
    popq    %rax
    popq    %rdi
    callq   *%rax
    .size thread_start, . - thread_start
