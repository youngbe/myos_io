	.file	"memset.c"
    .section    .text.memset_i386,"ax",@progbits
	.p2align 2
	.globl	memset_i386
	.type	memset_i386, @function
memset_i386:
	pushl	%edi
	movl	16(%esp), %ecx
	testl	%ecx, %ecx
	je	.L4
	movl	12(%esp), %eax
	movl	8(%esp), %edi
	rep stosb
.L4:
	movl	8(%esp), %eax
	popl	%edi
	ret
	.size	memset_i386, .-memset_i386
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
