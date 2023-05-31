	.text
	.file	"mtx_trylock_c.c"
	.section	.text.mtx_trylock,"ax",@progbits
	.globl	mtx_trylock                     # -- Begin function mtx_trylock
	.p2align	4, 0x90
	.type	mtx_trylock,@function
mtx_trylock:                            # @mtx_trylock
.Lmtx_trylock$local:
	.type	.Lmtx_trylock$local,@function
# %bb.0:
	leaq	8(%rdi), %rdx
	#APP
	#NO_APP
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_1
# %bb.4:
	#APP
	movq	%gs:0, %rdx
	#NO_APP
	movl	$1, %ecx
	cmpq	%rdx, %rax
	je	.LBB0_5
.LBB0_8:
	movl	%ecx, %eax
	retq
.LBB0_1:
	movl	$1, %ecx
	cmpq	$0, 16(%rdi)
	jne	.LBB0_8
# %bb.2:
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, 16(%rdi)
	jne	.LBB0_8
# %bb.3:
	#APP
	movq	%gs:0, %rax
	#NO_APP
	movq	%rax, (%rdi)
	xorl	%ecx, %ecx
	movl	%ecx, %eax
	retq
.LBB0_5:
	movq	24(%rdi), %rax
	movl	$2, %ecx
	incq	%rax
	cmpq	$2, %rax
	jb	.LBB0_8
# %bb.6:
	movq	%rax, 24(%rdi)
	xorl	%ecx, %ecx
	movl	%ecx, %eax
	retq
.Lfunc_end0:
	.size	mtx_trylock, .Lfunc_end0-mtx_trylock
	.size	.Lmtx_trylock$local, .Lfunc_end0-mtx_trylock
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
