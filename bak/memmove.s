	.file	"memmove.c"
	.section .text.memmove,"ax",@progbits
	.p2align 4
	.globl	memmove
	.type	memmove, @function
memmove:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	andl	$-32, %esp
	subl	$32, %esp
	movl	8(%ebp), %ecx
	movl	12(%ebp), %eax
	movl	16(%ebp), %edx
	cmpl	%eax, %ecx
	jb	.L5
	leal	(%eax,%edx), %ebx
	cmpl	%ebx, %ecx
	jb	.L94
.L5:
	testl	%edx, %edx
	je	.L4
	leal	-1(%edx), %ebx
	cmpl	$14, %ebx
	jbe	.L7
	leal	1(%eax), %edi
	movl	%ecx, %esi
	subl	%edi, %esi
	cmpl	$30, %esi
	ja	.L95
.L7:
	movl	%ecx, %ebx
	leal	(%edx,%eax), %edi
	.p2align 4,,10
	.p2align 3
.L13:
	movzbl	(%eax), %edx
	addl	$1, %eax
	addl	$1, %ebx
	movb	%dl, -1(%ebx)
	cmpl	%eax, %edi
	jne	.L13
.L4:
	leal	-12(%ebp), %esp
	movl	%ecx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L95:
	cmpl	$30, %ebx
	jbe	.L24
	movl	%edx, %edi
	movl	%eax, %ebx
	movl	%ecx, %esi
	andl	$-32, %edi
	addl	%eax, %edi
	.p2align 4,,10
	.p2align 3
.L9:
	vmovdqu	(%ebx), %ymm0
	addl	$32, %ebx
	addl	$32, %esi
	vmovdqu	%ymm0, -32(%esi)
	cmpl	%edi, %ebx
	jne	.L9
	movl	%edx, %esi
	andl	$-32, %esi
	testb	$31, %dl
	je	.L90
	movl	%edx, %ebx
	subl	%esi, %ebx
	leal	-1(%ebx), %edi
	cmpl	$14, %edi
	jbe	.L96
	vzeroupper
.L8:
	vmovdqu	(%eax,%esi), %xmm1
	movl	%ebx, %edi
	andl	$-16, %edi
	vmovdqu	%xmm1, (%ecx,%esi)
	addl	%edi, %esi
	andl	$15, %ebx
	je	.L4
.L11:
	movzbl	(%eax,%esi), %ebx
	movb	%bl, (%ecx,%esi)
	leal	1(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	1(%eax,%esi), %ebx
	movb	%bl, 1(%ecx,%esi)
	leal	2(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	2(%eax,%esi), %ebx
	movb	%bl, 2(%ecx,%esi)
	leal	3(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	3(%eax,%esi), %ebx
	movb	%bl, 3(%ecx,%esi)
	leal	4(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	4(%eax,%esi), %ebx
	movb	%bl, 4(%ecx,%esi)
	leal	5(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	5(%eax,%esi), %ebx
	movb	%bl, 5(%ecx,%esi)
	leal	6(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	6(%eax,%esi), %ebx
	movb	%bl, 6(%ecx,%esi)
	leal	7(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	7(%eax,%esi), %ebx
	movb	%bl, 7(%ecx,%esi)
	leal	8(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	8(%eax,%esi), %ebx
	movb	%bl, 8(%ecx,%esi)
	leal	9(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	9(%eax,%esi), %ebx
	movb	%bl, 9(%ecx,%esi)
	leal	10(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	10(%eax,%esi), %ebx
	movb	%bl, 10(%ecx,%esi)
	leal	11(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	11(%eax,%esi), %ebx
	movb	%bl, 11(%ecx,%esi)
	leal	12(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	12(%eax,%esi), %ebx
	movb	%bl, 12(%ecx,%esi)
	leal	13(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	13(%eax,%esi), %ebx
	movb	%bl, 13(%ecx,%esi)
	leal	14(%esi), %ebx
	cmpl	%edx, %ebx
	jnb	.L4
	movzbl	14(%eax,%esi), %eax
	movb	%al, 14(%ecx,%esi)
	jmp	.L4
	.p2align 4,,10
	.p2align 3
.L94:
	cmpl	%ecx, %eax
	jnb	.L4
	testl	%edx, %edx
	je	.L4
	leal	-1(%edx), %ebx
	cmpl	$14, %ebx
	jbe	.L22
	leal	-2(%eax,%edx), %edi
	movl	%ecx, %esi
	subl	%edi, %esi
	leal	30(%esi,%ebx), %esi
	cmpl	$30, %esi
	jbe	.L22
	jmp	.L97
	.p2align 4,,10
	.p2align 3
.L98:
	subl	$1, %ebx
.L22:
	movzbl	(%eax,%ebx), %edx
	movb	%dl, (%ecx,%ebx)
	testl	%ebx, %ebx
	jne	.L98
	jmp	.L4
.L97:
	cmpl	$30, %ebx
	jbe	.L25
	leal	-32(%edx), %esi
	leal	(%esi,%ecx), %edi
	leal	(%eax,%esi), %ebx
	movl	%edi, 28(%esp)
	movl	%edx, %edi
	movl	%ebx, %esi
	andl	$-32, %edi
	subl	%edi, %esi
	movl	%esi, %edi
	.p2align 4,,10
	.p2align 3
.L17:
	movl	28(%esp), %esi
	vmovdqu	(%ebx), %ymm2
	subl	$32, %ebx
	vmovdqu	%ymm2, (%esi)
	subl	$32, %esi
	movl	%esi, 28(%esp)
	cmpl	%ebx, %edi
	jne	.L17
	movl	%edx, %ebx
	andl	$31, %ebx
	andl	$31, %edx
	je	.L90
	leal	-1(%ebx), %esi
	movl	%ebx, %edx
	cmpl	$14, %esi
	jbe	.L99
	vzeroupper
.L16:
	vmovdqu	-16(%eax,%edx), %xmm3
	movl	%edx, %esi
	andl	$-16, %esi
	vmovdqu	%xmm3, -16(%ecx,%edx)
	subl	%esi, %ebx
	andl	$15, %edx
	je	.L4
	leal	-1(%ebx), %esi
.L20:
	movzbl	-1(%eax,%ebx), %edx
	movb	%dl, -1(%ecx,%ebx)
	testl	%esi, %esi
	je	.L4
	movzbl	-2(%eax,%ebx), %edx
	movb	%dl, -2(%ecx,%ebx)
	cmpl	$2, %ebx
	je	.L4
	movzbl	-3(%eax,%ebx), %edx
	movb	%dl, -3(%ecx,%ebx)
	cmpl	$3, %ebx
	je	.L4
	movzbl	-4(%eax,%ebx), %edx
	movb	%dl, -4(%ecx,%ebx)
	cmpl	$4, %ebx
	je	.L4
	movzbl	-5(%eax,%ebx), %edx
	movb	%dl, -5(%ecx,%ebx)
	cmpl	$5, %ebx
	je	.L4
	movzbl	-6(%eax,%ebx), %edx
	movb	%dl, -6(%ecx,%ebx)
	cmpl	$6, %ebx
	je	.L4
	movzbl	-7(%eax,%ebx), %edx
	movb	%dl, -7(%ecx,%ebx)
	cmpl	$7, %ebx
	je	.L4
	movzbl	-8(%eax,%ebx), %edx
	movb	%dl, -8(%ecx,%ebx)
	cmpl	$8, %ebx
	je	.L4
	movzbl	-9(%eax,%ebx), %edx
	movb	%dl, -9(%ecx,%ebx)
	cmpl	$9, %ebx
	je	.L4
	movzbl	-10(%eax,%ebx), %edx
	movb	%dl, -10(%ecx,%ebx)
	cmpl	$10, %ebx
	je	.L4
	movzbl	-11(%eax,%ebx), %edx
	movb	%dl, -11(%ecx,%ebx)
	cmpl	$11, %ebx
	je	.L4
	movzbl	-12(%eax,%ebx), %edx
	movb	%dl, -12(%ecx,%ebx)
	cmpl	$12, %ebx
	je	.L4
	movzbl	-13(%eax,%ebx), %edx
	movb	%dl, -13(%ecx,%ebx)
	cmpl	$13, %ebx
	je	.L4
	movzbl	-14(%eax,%ebx), %edx
	movb	%dl, -14(%ecx,%ebx)
	cmpl	$14, %ebx
	je	.L4
	movzbl	-15(%eax,%ebx), %eax
	movb	%al, -15(%ecx,%ebx)
	jmp	.L4
.L24:
	movl	%edx, %ebx
	xorl	%esi, %esi
	jmp	.L8
.L96:
	vzeroupper
	jmp	.L11
.L90:
	vzeroupper
	jmp	.L4
.L99:
	vzeroupper
	jmp	.L20
.L25:
	movl	%edx, %ebx
	jmp	.L16
	.size	memmove, .-memmove
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
