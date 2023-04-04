	.file	"memset.c"
	.section .text.memset,"ax",@progbits
	.p2align 4
	.globl	memset
	.type	memset, @function
memset:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	16(%ebp), %edx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %ebx
	testl	%edx, %edx
	je	.L13
	leal	-1(%edx), %eax
	cmpl	$30, %eax
	jbe	.L10
	movl	%edx, %esi
	vmovd	%ebx, %xmm0
	movl	%ecx, %eax
	andl	$-32, %esi
	vpbroadcastb	%xmm0, %ymm0
	addl	%ecx, %esi
	.p2align 4,,10
	.p2align 3
.L4:
	vmovdqu	%ymm0, (%eax)
	addl	$32, %eax
	cmpl	%eax, %esi
	jne	.L4
	movl	%edx, %eax
	andl	$-32, %eax
	testb	$31, %dl
	je	.L22
	vzeroupper
.L3:
	movl	%edx, %esi
	subl	%eax, %esi
	leal	-1(%esi), %edi
	cmpl	$14, %edi
	jbe	.L8
	movl	%esi, %edi
	vmovd	%ebx, %xmm0
	andl	$-16, %edi
	vpbroadcastb	%xmm0, %xmm0
	vmovdqu	%xmm0, (%ecx,%eax)
	addl	%edi, %eax
	andl	$15, %esi
	je	.L13
.L8:
	leal	1(%eax), %esi
	movb	%bl, (%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	2(%eax), %esi
	movb	%bl, 1(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	3(%eax), %esi
	movb	%bl, 2(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	4(%eax), %esi
	movb	%bl, 3(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	5(%eax), %esi
	movb	%bl, 4(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	6(%eax), %esi
	movb	%bl, 5(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	7(%eax), %esi
	movb	%bl, 6(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	8(%eax), %esi
	movb	%bl, 7(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	9(%eax), %esi
	movb	%bl, 8(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	10(%eax), %esi
	movb	%bl, 9(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	11(%eax), %esi
	movb	%bl, 10(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	12(%eax), %esi
	movb	%bl, 11(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	13(%eax), %esi
	movb	%bl, 12(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	leal	14(%eax), %esi
	movb	%bl, 13(%ecx,%eax)
	cmpl	%edx, %esi
	jnb	.L13
	movb	%bl, 14(%ecx,%eax)
.L13:
	popl	%ebx
	movl	%ecx, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L10:
	xorl	%eax, %eax
	jmp	.L3
.L22:
	vzeroupper
	jmp	.L13
	.size	memset, .-memset
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
