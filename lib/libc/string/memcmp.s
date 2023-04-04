# 此文件引用自glibc源代码
# glibc-2.37 中的 sysdeps/x86_64/multiarch/memcmp-avx2-movbe.S
# 链接： https://elixir.bootlin.com/glibc/glibc-2.37/source/sysdeps/x86_64/multiarch/memcmp-avx2-movbe.S
# memcmp-avx2-movbe.S -> memcmp.s 编译命令： 在构建glibc时，运行make命令期间，找到编译 sysdeps/x86_64/multiarch/memcmp-avx2-movbe.S 文件的命令，将 -c 选项修改为 -E 选项，即可获得编译后的汇编文件
    .code64
    .section .text.memcmp,"ax",@progbits
    .globl memcmp
    .type memcmp,@function
    .align 1<<4
memcmp:
 cmp $32, %rdx
 jb .Lless_vec
 vmovdqu (%rsi), %ymm1
 vpcmpeqb (%rdi), %ymm1, %ymm1
 vpmovmskb %ymm1, %eax
 incl %eax
 jnz .Lreturn_vec_0
 cmpq $(32 * 2), %rdx
 jbe .Llast_1x_vec
 vmovdqu 32(%rsi), %ymm2
 vpcmpeqb 32(%rdi), %ymm2, %ymm2
 vpmovmskb %ymm2, %eax
 incl %eax
 jnz .Lreturn_vec_1
 cmpq $(32 * 4), %rdx
 jbe .Llast_2x_vec
 vmovdqu (32 * 2)(%rsi), %ymm3
 vpcmpeqb (32 * 2)(%rdi), %ymm3, %ymm3
 vpmovmskb %ymm3, %eax
 incl %eax
 jnz .Lreturn_vec_2
 vmovdqu (32 * 3)(%rsi), %ymm4
 vpcmpeqb (32 * 3)(%rdi), %ymm4, %ymm4
 vpmovmskb %ymm4, %ecx
 incl %ecx
 jnz .Lreturn_vec_3
 cmpq $(32 * 8), %rdx
 ja .Lmore_8x_vec
 vmovdqu -(32 * 4)(%rsi, %rdx), %ymm1
 vmovdqu -(32 * 3)(%rsi, %rdx), %ymm2
 leaq -(4 * 32)(%rdi, %rdx), %rdi
 leaq -(4 * 32)(%rsi, %rdx), %rsi
 vpcmpeqb (%rdi), %ymm1, %ymm1
 vpcmpeqb (32)(%rdi), %ymm2, %ymm2
 vmovdqu (32 * 2)(%rsi), %ymm3
 vpcmpeqb (32 * 2)(%rdi), %ymm3, %ymm3
 vmovdqu (32 * 3)(%rsi), %ymm4
 vpcmpeqb (32 * 3)(%rdi), %ymm4, %ymm4
 vpand %ymm1, %ymm2, %ymm5
 vpand %ymm3, %ymm4, %ymm6
 vpand %ymm5, %ymm6, %ymm7
 vpmovmskb %ymm7, %ecx
 incl %ecx
 jnz .Lreturn_vec_0_1_2_3
 vzeroupper
 ret
 .p2align 4
.Lreturn_vec_0:
 tzcntl %eax, %eax
 movzbl (%rsi, %rax), %ecx
 movzbl (%rdi, %rax), %eax
 subl %ecx, %eax
.Lreturn_vzeroupper:
 vzeroupper
 ret
 .p2align 4
.Lreturn_vec_1:
 tzcntl %eax, %eax
 movzbl 32(%rsi, %rax), %ecx
 movzbl 32(%rdi, %rax), %eax
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 4
.Lreturn_vec_2:
 tzcntl %eax, %eax
 movzbl (32 * 2)(%rsi, %rax), %ecx
 movzbl (32 * 2)(%rdi, %rax), %eax
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 5
.L8x_return_vec_0_1_2_3:
 addq %rdi, %rsi
.Lreturn_vec_0_1_2_3:
 vpmovmskb %ymm1, %eax
 incl %eax
 jnz .Lreturn_vec_0
 vpmovmskb %ymm2, %eax
 incl %eax
 jnz .Lreturn_vec_1
 vpmovmskb %ymm3, %eax
 incl %eax
 jnz .Lreturn_vec_2
.Lreturn_vec_3:
 tzcntl %ecx, %ecx
 movzbl (32 * 3)(%rdi, %rcx), %eax
 movzbl (32 * 3)(%rsi, %rcx), %ecx
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 4
.Lmore_8x_vec:
 leaq -(32 * 4)(%rdi, %rdx), %rdx
 subq %rdi, %rsi
 andq $-32, %rdi
 subq $-(32 * 4), %rdi
 .p2align 4
.Lloop_4x_vec:
 vmovdqu (%rsi, %rdi), %ymm1
 vpcmpeqb (%rdi), %ymm1, %ymm1
 vmovdqu 32(%rsi, %rdi), %ymm2
 vpcmpeqb 32(%rdi), %ymm2, %ymm2
 vmovdqu (32 * 2)(%rsi, %rdi), %ymm3
 vpcmpeqb (32 * 2)(%rdi), %ymm3, %ymm3
 vmovdqu (32 * 3)(%rsi, %rdi), %ymm4
 vpcmpeqb (32 * 3)(%rdi), %ymm4, %ymm4
 vpand %ymm1, %ymm2, %ymm5
 vpand %ymm3, %ymm4, %ymm6
 vpand %ymm5, %ymm6, %ymm7
 vpmovmskb %ymm7, %ecx
 incl %ecx
 jnz .L8x_return_vec_0_1_2_3
 subq $-(32 * 4), %rdi
 cmpq %rdx, %rdi
 jb .Lloop_4x_vec
 subq %rdx, %rdi
 cmpl $(32 * 3), %edi
 jae .L8x_last_1x_vec
 vmovdqu (32 * 2)(%rsi, %rdx), %ymm3
 cmpl $(32 * 2), %edi
 jae .L8x_last_2x_vec
 vmovdqu (%rsi, %rdx), %ymm1
 vpcmpeqb (%rdx), %ymm1, %ymm1
 vmovdqu 32(%rsi, %rdx), %ymm2
 vpcmpeqb 32(%rdx), %ymm2, %ymm2
 vpcmpeqb (32 * 2)(%rdx), %ymm3, %ymm3
 vmovdqu (32 * 3)(%rsi, %rdx), %ymm4
 vpcmpeqb (32 * 3)(%rdx), %ymm4, %ymm4
 vpand %ymm1, %ymm2, %ymm5
 vpand %ymm3, %ymm4, %ymm6
 vpand %ymm5, %ymm6, %ymm7
 vpmovmskb %ymm7, %ecx
 movq %rdx, %rdi
 incl %ecx
 jnz .L8x_return_vec_0_1_2_3
 vzeroupper
 ret
 .p2align 4
.L8x_last_2x_vec:
 vpcmpeqb (32 * 2)(%rdx), %ymm3, %ymm3
 vpmovmskb %ymm3, %eax
 incl %eax
 jnz .L8x_return_vec_2
 .p2align 4
.L8x_last_1x_vec:
 vmovdqu (32 * 3)(%rsi, %rdx), %ymm4
 vpcmpeqb (32 * 3)(%rdx), %ymm4, %ymm4
 vpmovmskb %ymm4, %eax
 incl %eax
 jnz .L8x_return_vec_3
 vzeroupper
 ret
 .p2align 4
.Llast_2x_vec:
 vmovdqu -(32 * 2)(%rsi, %rdx), %ymm1
 vpcmpeqb -(32 * 2)(%rdi, %rdx), %ymm1, %ymm1
 vpmovmskb %ymm1, %eax
 incl %eax
 jnz .Lreturn_vec_1_end
.Llast_1x_vec:
 vmovdqu -(32 * 1)(%rsi, %rdx), %ymm1
 vpcmpeqb -(32 * 1)(%rdi, %rdx), %ymm1, %ymm1
 vpmovmskb %ymm1, %eax
 incl %eax
 jnz .Lreturn_vec_0_end
 vzeroupper
 ret
 .p2align 4
.L8x_return_vec_2:
 subq $32, %rdx
.L8x_return_vec_3:
 tzcntl %eax, %eax
 addq %rdx, %rax
 movzbl (32 * 3)(%rsi, %rax), %ecx
 movzbl (32 * 3)(%rax), %eax
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 4
.Lreturn_vec_1_end:
 tzcntl %eax, %eax
 addl %edx, %eax
 movzbl -(32 * 2)(%rsi, %rax), %ecx
 movzbl -(32 * 2)(%rdi, %rax), %eax
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 4
.Lreturn_vec_0_end:
 tzcntl %eax, %eax
 addl %edx, %eax
 movzbl -32(%rsi, %rax), %ecx
 movzbl -32(%rdi, %rax), %eax
 subl %ecx, %eax
 vzeroupper
 ret
 .p2align 4
.Lless_vec:
 cmpl $1, %edx
 jbe .Lone_or_less
 movl %edi, %eax
 orl %esi, %eax
 andl $(4096 - 1), %eax
 cmpl $(4096 - 32), %eax
 jg .Lpage_cross_less_vec
 vmovdqu (%rsi), %ymm2
 vpcmpeqb (%rdi), %ymm2, %ymm2
 vpmovmskb %ymm2, %eax
 incl %eax
 bzhil %edx, %eax, %edx
 jnz .Lreturn_vec_0
 xorl %eax, %eax
 vzeroupper
 ret
 .p2align 4
.Lpage_cross_less_vec:
 cmpl $16, %edx
 jae .Lbetween_16_31
 cmpl $8, %edx
 jae .Lbetween_8_15
 cmpl $4, %edx
 jb .Lbetween_2_3
 movbe (%rdi), %eax
 movbe (%rsi), %ecx
 shlq $32, %rax
 shlq $32, %rcx
 movbe -4(%rdi, %rdx), %edi
 movbe -4(%rsi, %rdx), %esi
 orq %rdi, %rax
 orq %rsi, %rcx
 subq %rcx, %rax
 jnz .Lret_nonzero
 ret
 .p2align 4
.Lone_or_less:
 jb .Lzero
 movzbl (%rsi), %ecx
 movzbl (%rdi), %eax
 subl %ecx, %eax
 ret
 .p2align 4,, 5
.Lret_nonzero:
 sbbl %eax, %eax
 orl $1, %eax
 ret
 .p2align 4,, 2
.Lzero:
 xorl %eax, %eax
 ret
 .p2align 4
.Lbetween_8_15:
 movbe (%rdi), %rax
 movbe (%rsi), %rcx
 subq %rcx, %rax
 jnz .Lret_nonzero
 movbe -8(%rdi, %rdx), %rax
 movbe -8(%rsi, %rdx), %rcx
 subq %rcx, %rax
 jnz .Lret_nonzero
 ret
 .p2align 4,, 10
.Lbetween_16_31:
 vmovdqu (%rsi), %xmm2
 vpcmpeqb (%rdi), %xmm2, %xmm2
 vpmovmskb %xmm2, %eax
 subl $0xffff, %eax
 jnz .Lreturn_vec_0
 vmovdqu -16(%rsi, %rdx), %xmm2
 leaq -16(%rdi, %rdx), %rdi
 leaq -16(%rsi, %rdx), %rsi
 vpcmpeqb (%rdi), %xmm2, %xmm2
 vpmovmskb %xmm2, %eax
 subl $0xffff, %eax
 jnz .Lreturn_vec_0
 ret
 .p2align 4
.Lbetween_2_3:
 movzwl (%rdi), %eax
 movzwl (%rsi), %ecx
 bswap %eax
 bswap %ecx
 shrl %eax
 shrl %ecx
 movzbl -1(%rdi, %rdx), %edi
 movzbl -1(%rsi, %rdx), %esi
 orl %edi, %eax
 orl %esi, %ecx
 subl %ecx, %eax
 ret
 .size memcmp,.-memcmp
