    .section .text.abort_handler, "ax", @progbits
    .globl abort_handler
    .type abort_handler, @function
    .p2align 4, 0x90
abort_handler:
    cld
    movq    $0xb8000 + 16, %rdi
    movq    $80 * 25 * 2 - 16, %rcx
    xorl    %eax, %eax
    rep stosb
    movq    $'a' | (0xf << 8) | ('b' << 16) | (0xf << 24) | ('o' << 32) | (0xf << 40) | ('r' << 48) | (0xf << 56), %rax
    movq    %rax, 0xb8000
    movq    $'t' | (0xf << 8) | ('!' << 16) | (0xf << 24), %rbx
    movq    %rbx, 0xb8008
1:
    cli
    hlt
    jmp 1b
    .size abort_handler, . - abort_handler
