# 根据GCC和CLANG的文档，这两个编译器，即使是在-ffreestanding选项开启的情况下，仍然需要准备memcpy, memset, memmove, memcmp 四个函数
# 这意味着即使在-ffreestanding选项开启的情况下，编译器都有可能自动生成调用memcpy, memset, memmove, memcmp四个函数的代码，即使在C源代码中从未调用过这几个函数，更别提未开启-ffreestanding选项的情况了
# https://gcc.gnu.org/onlinedocs/gcc/Standards.html#C-Language
# https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-ffreestanding
# 本汇编文件实现了memcpy, memset, memmove, memcmp 四个函数，并且分在了不同section，可以使用-gc-sections进行链接，如果实际没有使用到这些函数，将会自动删除掉
# 在实现上，假设机器具有ERMS特性，这意味着，rep movsb/stosb的速度是很快的

    .code32
    .section .text.memcpy,"ax",@progbits
    .p2align 4
    .globl  memcpy
    .type   memcpy, @function
memcpy:
    pushl   %edi
    pushl   %esi
    movl    12(%esp), %edi
    movl    16(%esp), %esi
    movl    20(%esp), %ecx
    movl    %edi, %eax
    rep movsb
    popl    %esi
    popl    %edi
    retl
    .size   memcpy, .-memcpy

    .section .text.memmove,"ax",@progbits
    .p2align 4
    .globl  memmove
    .type   memmove, @function
memmove:
    pushl   %edi
    pushl   %esi
    movl    12(%esp), %edi
    movl    16(%esp), %esi
    movl    20(%esp), %ecx
    movl    %edi, %eax
    cmpl    %edi, %esi
    jge     1f
    movl    %esi, %edx
    addl    %ecx, %edx
    cmpl    %edi, %edx
    jg 2f
1:
    rep movsb
    popl    %esi
    popl    %edi
    retl
2:
    addl    %ecx, %edi
    decl    %edi
    addl    %ecx, %esi
    decl    %esi
    std
    rep movsb
    cld
    popl    %esi
    popl    %edi
    retl
    .size   memmove, .-memmove

    .section .text.memset,"ax",@progbits
    .p2align 4
    .globl  memset
    .type   memset, @function
memset:
    pushl   %edi
    movl    8(%esp), %edi
    movb    12(%esp), %al
    movl    16(%esp), %ecx
    movl    %edi, %edx
    rep stosb
    movl    %edx, %eax
    popl    %edi
    retl
    .size   memset, .-memset

    .section .text.memcmp,"ax",@progbits
    .p2align 4
    .globl  memcmp
    .type   memcmp, @function
memcmp:
    pushl	%edi
    pushl	%esi
    movl	12(%esp), %edi
    movl	16(%esp), %esi
    movl	20(%esp), %ecx
    xorl    %eax, %eax
    repe cmpsb
    setne	%al
    popl	%esi
    popl	%edi
    retl
    .size   memcmp, .-memcmp
