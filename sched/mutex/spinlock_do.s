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
	je	4f
	movq	%rsp, 8(%rax)
	cmpl	$0, (%rsp)
	jne	4f
	.p2align	4, 0x90
2:                                # =>This Inner Loop Header: Depth=1
	pause
	cmpl	$0, (%rsp)
	je	2b
4:
	callq	*%rdi
	movq	%rax, %rcx
	xorl	%esi, %esi
	movq	(%rbx), %rax
	cmpq	%rsp, %rax
	jne	7f
	lock		cmpxchgq	%rsi, (%rbx)
	jne	7f
	jmp	9f
	.p2align	4, 0x90
6:                                #   in Loop: Header=BB0_7 Depth=1
	pause
7:                                # =>This Inner Loop Header: Depth=1
	movq	8(%rsp), %rax
	testq	%rax, %rax
	je	6b
	movl	$1, (%rax)
9:
    sti
	movq	%rcx, %rax
	addq	$16, %rsp
	popq	%rbx
	retq
	.size	cli_spinlock_do, .-cli_spinlock_do


	.section	.text.spinlock_do,"ax",@progbits
	.globl	spinlock_do                 # -- Begin function spinlock_do
	.p2align	4, 0x90
	.type	spinlock_do,@function
spinlock_do:                        # @spinlock_do
	pushq	%rbx
	subq	$16, %rsp
	movq	%rsi, %rbx
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	movq	%rsp, %rax
	xchgq	%rax, (%rsi)
	testq	%rax, %rax
	je	4f
	movq	%rsp, 8(%rax)
	cmpl	$0, (%rsp)
	jne	4f
	.p2align	4, 0x90
2:                                # =>This Inner Loop Header: Depth=1
	pause
	cmpl	$0, (%rsp)
	je	2b
4:
	callq	*%rdi
	movq	%rax, %rcx
	xorl	%esi, %esi
	movq	(%rbx), %rax
	cmpq	%rsp, %rax
	jne	7f
	lock		cmpxchgq	%rsi, (%rbx)
	jne	7f
	jmp	9f
	.p2align	4, 0x90
6:                                #   in Loop: Header=BB0_7 Depth=1
	pause
7:                                # =>This Inner Loop Header: Depth=1
	movq	8(%rsp), %rax
	testq	%rax, %rax
	je	6b
	movl	$1, (%rax)
9:
	movq	%rcx, %rax
	addq	$16, %rsp
	popq	%rbx
	retq
	.size	spinlock_do, .-spinlock_do
