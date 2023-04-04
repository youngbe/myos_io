    .section .text.spurious_isr, "ax", @progbits
    .globl spurious_isr
    .p2align    4, 0x90
    .type   spurious_isr,@function
spurious_isr:
    iretq
    .size spurious_isr, . - spurious_isr
