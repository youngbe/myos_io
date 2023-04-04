	.file	"memcmp.c"
    .section    .text.memcmp_i386,"ax",@progbits
	.p2align 2
	.globl	memcmp_i386
	.type	memcmp_i386, @function
memcmp_i386:
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
	movzbl	%al, %eax
	popl	%esi
	popl	%edi
	ret
	.size	memcmp_i386, .-memcmp_i386
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
