	.section	.text.mtx_trylock,"ax",@progbits
	.globl	mtx_trylock                     # -- Begin function mtx_trylock
	.p2align	4, 0x90
	.type	mtx_trylock,@function
mtx_trylock:                            # @mtx_trylock
	movq	%gs:0, %rdx
	#NO_APP
	cmpq	%rdx, (%rdi)
	je	.LBB0_1
# %bb.3:
	leaq	8(%rdi), %rsi
	#APP
	#NO_APP
	#APP
	pushfq
	popq	%rax
	#NO_APP
	testl	$512, %eax                      # imm = 0x200
	jne	.LBB0_4
# %bb.6:
	movl	$1, %ecx
	cmpq	$0, 16(%rdi)
	je	.LBB0_7
.LBB0_10:
	movl	%ecx, %eax
	retq
.LBB0_1:
	movq	24(%rdi), %rax
	movl	$2, %ecx
	incq	%rax
	cmpq	$2, %rax
	jb	.LBB0_10
# %bb.2:
	movq	%rax, 24(%rdi)
	xorl	%eax, %eax
	retq
.LBB0_4:
	#APP
	cli
	#NO_APP
	cmpq	$0, 16(%rdi)
	je	.LBB0_5
# %bb.9:
	#APP
	sti
	#NO_APP
	movl	$1, %ecx
	movl	%ecx, %eax
	retq
.LBB0_7:
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 16(%rdi)
	jne	.LBB0_10
	jmp	.LBB0_8
.LBB0_5:
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 16(%rdi)
	sti
	movl	$1, %ecx
	jne	.LBB0_10
.LBB0_8:
	movq	%rdx, (%rdi)
	xorl	%eax, %eax
	retq
.Lfunc_end0:
	.size	mtx_trylock, .Lfunc_end0-mtx_trylock
