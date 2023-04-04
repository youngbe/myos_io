	.file	"memcpy.c"
	.section .text.memcpy,"ax",@progbits
	.p2align 4
	.globl	memcpy
	.type	memcpy, @function
memcpy:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%ebp), %edx
	movl	12(%ebp), %eax
	testl	%edx, %edx
	je	.L2
	leal	-1(%edx), %ecx
	cmpl	$14, %ecx
	jbe	.L3
	movl	8(%ebp), %esi
	leal	1(%eax), %edi
	subl	%edi, %esi
	cmpl	$30, %esi
	ja	.L27
.L3:
	movl	8(%ebp), %ecx
	addl	%eax, %edx
	.p2align 4,,10
	.p2align 3
.L9:
	movzbl	(%eax), %ebx
	addl	$1, %eax
	addl	$1, %ecx
	movb	%bl, -1(%ecx)
	cmpl	%eax, %edx
	jne	.L9
.L2:
	popl	%ebx
	movl	8(%ebp), %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L27:
	cmpl	$30, %ecx
	jbe	.L11
	movl	%edx, %edi
	movl	8(%ebp), %esi
	movl	%eax, %ecx
	andl	$-32, %edi
	addl	%eax, %edi
	.p2align 4,,10
	.p2align 3
.L5:
	vmovdqu	(%ecx), %ymm0
	addl	$32, %ecx
	addl	$32, %esi
	vmovdqu	%ymm0, -32(%esi)
	cmpl	%edi, %ecx
	jne	.L5
	movl	%edx, %ecx
	andl	$-32, %ecx
	testb	$31, %dl
	je	.L23
	movl	%edx, %esi
	subl	%ecx, %esi
	leal	-1(%esi), %edi
	cmpl	$14, %edi
	jbe	.L28
	vzeroupper
.L4:
	vmovdqu	(%eax,%ecx), %xmm1
	movl	8(%ebp), %edi
	vmovdqu	%xmm1, (%edi,%ecx)
	movl	%esi, %edi
	andl	$-16, %edi
	addl	%edi, %ecx
	andl	$15, %esi
	je	.L2
.L7:
	movzbl	(%eax,%ecx), %ebx
	movl	8(%ebp), %edi
	leal	1(%ecx), %esi
	movb	%bl, (%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	1(%eax,%ecx), %ebx
	leal	2(%ecx), %esi
	movb	%bl, 1(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	2(%eax,%ecx), %ebx
	leal	3(%ecx), %esi
	movb	%bl, 2(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	3(%eax,%ecx), %ebx
	leal	4(%ecx), %esi
	movb	%bl, 3(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	4(%eax,%ecx), %ebx
	leal	5(%ecx), %esi
	movb	%bl, 4(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	5(%eax,%ecx), %ebx
	leal	6(%ecx), %esi
	movb	%bl, 5(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	6(%eax,%ecx), %ebx
	leal	7(%ecx), %esi
	movb	%bl, 6(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	7(%eax,%ecx), %ebx
	leal	8(%ecx), %esi
	movb	%bl, 7(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	8(%eax,%ecx), %ebx
	leal	9(%ecx), %esi
	movb	%bl, 8(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	9(%eax,%ecx), %ebx
	leal	10(%ecx), %esi
	movb	%bl, 9(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	10(%eax,%ecx), %ebx
	leal	11(%ecx), %esi
	movb	%bl, 10(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	11(%eax,%ecx), %ebx
	leal	12(%ecx), %esi
	movb	%bl, 11(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	12(%eax,%ecx), %ebx
	leal	13(%ecx), %esi
	movb	%bl, 12(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	13(%eax,%ecx), %ebx
	leal	14(%ecx), %esi
	movb	%bl, 13(%edi,%ecx)
	cmpl	%edx, %esi
	jnb	.L2
	movzbl	14(%eax,%ecx), %eax
	movb	%al, 14(%edi,%ecx)
	jmp	.L2
.L11:
	movl	%edx, %esi
	xorl	%ecx, %ecx
	jmp	.L4
.L28:
	vzeroupper
	jmp	.L7
.L23:
	vzeroupper
	jmp	.L2
	.size	memcpy, .-memcpy
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
