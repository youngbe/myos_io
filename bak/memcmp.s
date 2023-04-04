	.file	"memcmp.c"
	.section .text.memcmp,"ax",@progbits
	.p2align 4
	.globl	memcmp
	.type	memcmp, @function
memcmp:
	pushl	%edi
	pushl	%esi
	movl	12(%esp), %edi
	movl	16(%esp), %esi
	movl	20(%esp), %ecx
#APP
# 26 "memcmp.c" 1
	repe; cmpsb
	/* output condition code nz*/

# 0 "" 2
#NO_APP
	setne	%al
	popl	%esi
	popl	%edi
	movzbl	%al, %eax
	ret
	.size	memcmp, .-memcmp
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
