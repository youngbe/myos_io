    .code64
    .section .text.memcpy,"ax",@progbits
    .p2align 4
    .globl  memcpy
    .type   memcpy, @function
memcpy:
    movq %rdi, %rax
    movq %rdx, %rcx
    rep movsb
    retq
    .size   memcpy, .-memcpy

    .section .text.memmove,"ax",@progbits
    .p2align 4
    .globl  memmove
    .type   memmove, @function
memmove:
    movq %rdi, %rax
    movq %rdx, %rcx

    cmpq %rdi, %rsi
    jge 1f
    movq %rsi, %r8
    addq %rdx, %r8
    cmpq %rdi, %r8
    jg 2f
1:
    rep movsb
    retq
2:
    leaq -1(%rsi, %rdx), %rsi
    leaq -1(%rdi, %rdx), %rdi
    std
    rep movsb
    cld
    retq
    .size   memmove, .-memmove

    .section .text.memset,"ax",@progbits
    .p2align 4
    .globl  memset
    .type   memset, @function
memset:
    movq %rdi,%r9
    movb %sil,%al
    movq %rdx,%rcx
    rep stosb
    movq %r9,%rax
    retq
    .size   memset, .-memset

    .section .text.memcmp,"ax",@progbits
    .p2align 4
    .globl  memcmp
    .type   memcmp, @function
memcmp:
    xorq    %rax, %rax
    movq    %rdx, %rcx
    repe cmpsb
    setne   %al
    retq
    .size   memcmp, .-memcmp
