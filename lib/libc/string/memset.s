# 此文件引用自glibc源代码
# glibc-2.37 中的 sysdeps/x86_64/multiarch/memset-avx2-unaligned-erms.S
# 链接：https://elixir.bootlin.com/glibc/glibc-2.37/source/sysdeps/x86_64/multiarch/memset-avx2-unaligned-erms.S
# memset-avx2-unaligned-erms.S -> memset.s 编译命令： 在构建glibc时，运行make命令期间，找到编译 sysdeps/x86_64/multiarch/memset-avx2-unaligned-erms.S 文件的命令，将 -c 选项修改为 -E 选项，即可获得编译后的汇编文件 
    .set __x86_rep_stosb_threshold, 0x800
    .code64
    .section .text.memset, "ax", @progbits
    .globl memset
    .type memset,@function
    .align 1<<6
memset:
 vmovd %esi, %xmm0
 movq %rdi, %rax
 cmp $32, %rdx
 jb .Lless_vec
 vpbroadcastb %xmm0, %ymm0
 cmp $(32 * 2), %rdx
 ja .Lstosb_more_2x_vec
 vmovdqu %ymm0, (%rdi)
 vmovdqu %ymm0, (32 * -1)(%rdi, %rdx)
 vzeroupper
 ret
 .p2align 4,, 4
.Llast_2x_vec:
 vmovdqu %ymm0, (32 * -2)(%rdi)
 vmovdqu %ymm0, (32 * -1)(%rdi)
 vzeroupper
 ret
 .p2align 4
.Lstosb_more_2x_vec:
 cmp $__x86_rep_stosb_threshold, %rdx
 ja .Lstosb_local
.Lmore_2x_vec:
 vmovdqu %ymm0, (%rdi)
 vmovdqu %ymm0, (32 * 1)(%rdi)
 addq %rdx, %rdi
 cmpq $(32 * 4), %rdx
 jbe .Llast_2x_vec
 vmovdqu %ymm0, (32 * 2)(%rax)
 vmovdqu %ymm0, (32 * 3)(%rax)
 addq $-(32 * 4), %rdi
 cmpq $(32 * 8), %rdx
 jbe .Llast_4x_vec
 leaq (32 * 4)(%rax), %rdx
 andq $(32 * -2), %rdx
 .p2align 4
.Lloop:
 vmovdqa %ymm0, (0)(%rdx)
 vmovdqa %ymm0, (32 + (0))(%rdx)
 vmovdqa %ymm0, (32 * 2 + (0))(%rdx)
 vmovdqa %ymm0, (32 * 3 + (0))(%rdx)
 subq $-(32 * 4), %rdx
 cmpq %rdi, %rdx
 jb .Lloop
 .p2align 4,, 4
.Llast_4x_vec:
 vmovdqu %ymm0, (0)(%rdi)
 vmovdqu %ymm0, (32 + (0))(%rdi)
 vmovdqu %ymm0, (32 * 2 + (0))(%rdi)
 vmovdqu %ymm0, (32 * 3 + (0))(%rdi)
.Lreturn_vzeroupper:
 vzeroupper
 ret
 .p2align 4,, 10
.Lstosb_local:
 movzbl %sil, %eax
 mov %rdx, %rcx
 mov %rdi, %rdx
 rep stosb
 mov %rdx, %rax
 vzeroupper
 ret
 .p2align 4
.Lless_vec:
 vpbroadcastb %xmm0, %xmm0
.Lless_vec_from_wmemset:
.Lcross_page:
 cmpl $16, %edx
 jge .Lbetween_16_31
 cmpl $8, %edx
 jge .Lbetween_8_15
 cmpl $4, %edx
 jge .Lbetween_4_7
 cmpl $1, %edx
 jg .Lbetween_2_3
 jl .Lbetween_0_0
 movb %sil, (%rdi)
.Lbetween_0_0:
 ret
 .p2align 4,, (2 * (4) + (1) + 1)
.Lbetween_16_31:
 vmovdqu %xmm0, (%rdi)
 vmovdqu %xmm0, -16(%rdi, %rdx)
 ret
 .p2align 4,, (2 * (3 + 1) + (1) + 1)
.Lbetween_8_15:
 vmovq %xmm0, (%rdi)
 vmovq %xmm0, -8(%rdi, %rdx)
 ret
 .p2align 4,, (2 * (2 << 1) + (1) + 1)
.Lbetween_4_7:
 vmovd %xmm0, (%rdi)
 vmovd %xmm0, -4(%rdi, %rdx)
 ret
 .p2align 4,, 4 * 1 + (2 * (3) + (1) + 1)
.Lbetween_2_3:
 movb %sil, (%rdi)
 movb %sil, 1(%rdi)
 movb %sil, -1(%rdi, %rdx)
 ret
.size memset,.-memset
