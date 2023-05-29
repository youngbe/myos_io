    .section .text.cli_do,"ax",@progbits
    .globl cli_do
    .type cli_do, @function
    .p2align 4, 0x90
cli_do:
    subq    $8, %rsp
    cli
    callq   *%rdi
    sti
    retq
    .size cli_do, . - cli_do
