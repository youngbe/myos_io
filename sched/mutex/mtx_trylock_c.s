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
	#APP
	movq	%gs:0, %rax
	#NO_APP
	leaq	8(%rax), %rdx
	#APP
	#NO_APP
	movq	$0, 8(%rax)
	#APP
	pushfq
	popq	%rsi
	#NO_APP
	testl	$512, %esi                      # imm = 0x200
	jne	.LBB0_1
# %bb.6:
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_7
# %bb.3:
	movl	$1, %ecx
	cmpq	%rax, %rdx
	je	.LBB0_4
.LBB0_12:
	movl	%ecx, %eax
	retq
.LBB0_1:
	#APP
	cli
	#NO_APP
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_7
# %bb.2:
	#APP
	sti
	#NO_APP
	movl	$1, %ecx
	cmpq	%rax, %rdx
	jne	.LBB0_12
.LBB0_4:
	movq	16(%rdi), %rax
	movl	$2, %ecx
	incq	%rax
	cmpq	$2, %rax
	jb	.LBB0_12
# %bb.5:
	movq	%rax, 16(%rdi)
	xorl	%eax, %eax
	retq
.LBB0_7:
	movq	8(%rdi), %rax
	movl	$1, %ecx
	testq	%rax, %rax
	jne	.LBB0_10
# %bb.8:
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, 8(%rdi)
	jne	.LBB0_10
# %bb.9:
	movq	%rdx, (%rdi)
	xorl	%ecx, %ecx
.LBB0_10:
	testl	$512, %esi                      # imm = 0x200
	je	.LBB0_12
# %bb.11:
	#APP
	sti
	#NO_APP
	movl	%ecx, %eax
	retq
.Lfunc_end0:
	.size	mtx_trylock, .Lfunc_end0-mtx_trylock
	.size	.Lmtx_trylock$local, .Lfunc_end0-mtx_trylock
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
