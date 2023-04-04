	.file	"memcpy.c"
    .section    .text.memcpy_i386,"ax",@progbits
	.p2align 2
	.globl	memcpy_i386
	.type	memcpy_i386, @function
memcpy_i386:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	20(%esp), %ebp
	movl	24(%esp), %eax
	movl	28(%esp), %ecx
	testl	%ecx, %ecx
	je	.L2
	leal	-1(%ecx), %edx
	cmpl	$9, %edx
	jbe	.L3
	movl	%ebp, %ebx
	orl	%eax, %ebx
	movl	%ebx, %edx
	andl	$3, %edx
	jne	.L3
	leal	1(%eax), %esi
	movl	%ebp, %ebx
	subl	%esi, %ebx
	cmpl	$2, %ebx
	jbe	.L3
	movl	%eax, %edx
	movl	%ebp, %esi
	movl	%ecx, %edi
	andl	$-4, %edi
	addl	%eax, %edi
	.p2align 2
.L4:
	movl	(%edx), %ebx
	movl	%ebx, (%esi)
	addl	$4, %edx
	addl	$4, %esi
	cmpl	%edi, %edx
	jne	.L4
	movl	%ecx, %edx
	andl	$-4, %edx
	testb	$3, %cl
	je	.L2
	movb	(%eax,%edx), %bl
	movb	%bl, 0(%ebp,%edx)
	leal	1(%edx), %esi
	cmpl	%ecx, %esi
	jnb	.L2
	movb	1(%eax,%edx), %bl
	movb	%bl, 1(%ebp,%edx)
	addl	$2, %edx
	cmpl	%ecx, %edx
	jnb	.L2
	movb	(%eax,%edx), %al
	movb	%al, 0(%ebp,%edx)
.L2:
	movl	%ebp, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 2
.L3:
	movl	%ebp, %edx
	addl	%eax, %ecx
	.p2align 2
.L6:
	movb	(%eax), %bl
	movb	%bl, (%edx)
	incl	%eax
	incl	%edx
	cmpl	%ecx, %eax
	jne	.L6
	movl	%ebp, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	memcpy_i386, .-memcpy_i386
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
