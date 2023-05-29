	.text
	.file	"cli_spinlock_do_c.c"
	.section	.text.cli_spinlock_do,"ax",@progbits
	.globl	cli_spinlock_do                 # -- Begin function cli_spinlock_do
	.p2align	4, 0x90
	.type	cli_spinlock_do,@function
cli_spinlock_do:                        # @cli_spinlock_do
.Lcli_spinlock_do$local:
	.type	.Lcli_spinlock_do$local,@function
# %bb.0:
	pushq	%r14
	pushq	%rbx
	subq	$24, %rsp
	movq	%rsi, %rbx
	#APP
	#NO_APP
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	movq	%rsp, %r14
	movq	%r14, %rax
	xchgq	%rax, (%rsi)
	testq	%rax, %rax
	je	.LBB0_4
# %bb.1:
	movq	%r14, 8(%rax)
	cmpl	$0, (%rsp)
	jne	.LBB0_4
	.p2align	4, 0x90
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	cmpl	$0, (%rsp)
	je	.LBB0_2
.LBB0_4:
	movq	%rbx, %rsi
	callq	*%rdi
	movq	%rax, %rcx
	movq	(%rbx), %rax
	cmpq	%r14, %rax
	jne	.LBB0_7
# %bb.5:
	xorl	%esi, %esi
	movq	%rsp, %rax
	lock		cmpxchgq	%rsi, (%rbx)
	jne	.LBB0_7
	jmp	.LBB0_9
	.p2align	4, 0x90
.LBB0_6:                                #   in Loop: Header=BB0_7 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_7:                                # =>This Inner Loop Header: Depth=1
	movq	8(%rsp), %rax
	testq	%rax, %rax
	je	.LBB0_6
# %bb.8:
	movl	$1, (%rax)
.LBB0_9:
	movq	%rcx, %rax
	addq	$24, %rsp
	popq	%rbx
	popq	%r14
	retq
.Lfunc_end0:
	.size	cli_spinlock_do, .Lfunc_end0-cli_spinlock_do
	.size	.Lcli_spinlock_do$local, .Lfunc_end0-cli_spinlock_do
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
