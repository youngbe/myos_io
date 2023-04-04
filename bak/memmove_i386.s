	.file	"memmove.c"
    .section    .text.memmove_i386,"ax",@progbits
	.p2align 2
	.globl	memmove_i386
	.type	memmove_i386, @function
memmove_i386:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	20(%esp), %ebx
	movl	24(%esp), %eax
	movl	28(%esp), %ebp
	cmpl	%eax, %ebx
	jb	.L5
	leal	(%eax,%ebp), %ecx
	cmpl	%ecx, %ebx
	jb	.L31
.L5:
	testl	%ebp, %ebp
	je	.L4
	leal	-1(%ebp), %ecx
	cmpl	$9, %ecx
	jbe	.L7
	movl	%eax, %ecx
	orl	%ebx, %ecx
	andl	$3, %ecx
	jne	.L7
	leal	1(%eax), %esi
	movl	%ebx, %ecx
	subl	%esi, %ecx
	cmpl	$2, %ecx
	jbe	.L7
	movl	%eax, %ecx
	movl	%ebx, %esi
	movl	%ebp, %edi
	andl	$-4, %edi
	addl	%eax, %edi
	.p2align 2
.L8:
	movl	(%ecx), %edx
	movl	%edx, (%esi)
	addl	$4, %ecx
	addl	$4, %esi
	cmpl	%edi, %ecx
	jne	.L8
	movl	%ebp, %edx
	andl	$-4, %edx
	movl	%edx, %ecx
	testl	$3, %ebp
	je	.L4
	movb	(%eax,%edx), %dl
	movb	%dl, (%ebx,%ecx)
	leal	1(%ecx), %esi
	cmpl	%ebp, %esi
	jnb	.L4
	movb	1(%eax,%ecx), %dl
	movb	%dl, 1(%ebx,%ecx)
	addl	$2, %ecx
	cmpl	%ebp, %ecx
	jnb	.L4
	movb	(%eax,%ecx), %al
	movb	%al, (%ebx,%ecx)
.L4:
	movl	%ebx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L31:
	cmpl	%ebx, %eax
	jnb	.L4
	testl	%ebp, %ebp
	je	.L4
	.p2align 2
.L12:
	decl	%ebp
	movb	(%eax,%ebp), %cl
	movb	%cl, (%ebx,%ebp)
	jne	.L12
	movl	%ebx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L7:
	movl	%ebx, %ecx
	leal	0(%ebp,%eax), %edi
	.p2align 2
.L10:
	movb	(%eax), %dl
	movb	%dl, (%ecx)
	incl	%eax
	incl	%ecx
	cmpl	%edi, %eax
	jne	.L10
	movl	%ebx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	memmove_i386, .-memmove_i386
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
