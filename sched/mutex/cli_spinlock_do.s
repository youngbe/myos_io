	.section	.text.cli_spinlock_do,"ax",@progbits
	.globl	cli_spinlock_do                 # -- Begin function cli_spinlock_do
	.p2align	4, 0x90
	.type	cli_spinlock_do,@function
cli_spinlock_do:                        # @cli_spinlock_do
	pushq	%rbx
	subq	$16, %rsp
	movq	%rsi, %rbx
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	movq	%rsp, %rax
    cli
	xchgq	%rax, (%rsi)
	testq	%rax, %rax
	je	.LBB0_4
	movq	%rsp, 8(%rax)
	cmpl	$0, (%rsp)
	jne	.LBB0_4
	.p2align	4, 0x90
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
	pause
	cmpl	$0, (%rsp)
	je	.LBB0_2
.LBB0_4:
	callq	*%rdi
	movq	%rax, %rcx
	xorl	%esi, %esi
	movq	(%rbx), %rax
	cmpq	%rsp, %rax
	jne	.LBB0_7
	lock		cmpxchgq	%rsi, (%rbx)
	jne	.LBB0_7
	jmp	.LBB0_9
	.p2align	4, 0x90
.LBB0_6:                                #   in Loop: Header=BB0_7 Depth=1
	pause
.LBB0_7:                                # =>This Inner Loop Header: Depth=1
	movq	8(%rsp), %rax
	testq	%rax, %rax
	je	.LBB0_6
	movl	$1, (%rax)
.LBB0_9:
    sti
	movq	%rcx, %rax
	addq	$16, %rsp
	popq	%rbx
	retq
.Lfunc_end0:
	.size	cli_spinlock_do, .Lfunc_end0-cli_spinlock_do
