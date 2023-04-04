    .text
    .globl empty_loop
    .p2align    4, 0x90
    .type   empty_loop,@function
empty_loop:
    hlt
    jmp empty_loop
    .size empty_loop, . - empty_loop
