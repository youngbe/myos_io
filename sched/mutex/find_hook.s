// 这个函数用于已经切换至空线程 （栈换过去了， Core_Data的running_thread和current_proc也设置好了，进程的线程数量也加一了，即这个时候可以sti正常跑，但是咱们还是看看有没有新线程，然后再jmp empty_loop的情况）
# 进入之前，设置%rdx == current_proc
# 设置%rsp为栈顶-16
# 进入之前sti
	.section	.text.find_hook,"ax",@progbits
	.globl	find_hook                       # -- Begin function find_hook
	.p2align	4, 0x90
	.type	find_hook,@function
find_hook:                              # @find_hook
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	cli
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	.p2align	4, 0x90
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	testq	%rax, %rax
	je	.LBB0_14
	leaq	-1(%rax), %rcx
	lock		cmpxchgq	%rcx, schedulable_threads_num(%rip)
	jne	.LBB0_1
	movq	%rsp, %rax
	movq	%rax, %rcx
	xchgq	%rcx, schedulable_threads_lock(%rip)
	testq	%rcx, %rcx
	je	.LBB0_5
	movq	%rax, 8(%rcx)
	xorl	%eax, %eax
	.p2align	4, 0x90
.Ltmp0:
	cmpl	(%rsp), %eax
	jne	.Ltmp1
	pause
	jmp	.Ltmp0
.Ltmp1:
.LBB0_5:
    # spin lock end

	movq	schedulable_threads(%rip), %rcx
	movq	(%rcx), %rax
	cmpq	%rcx, %rax
	jne	.LBB0_7
	xorl	%eax, %eax
	jmp	.LBB0_8
.LBB0_7:
	movq	8(%rcx), %rsi
	movq	%rsi, 8(%rax)
	movq	8(%rcx), %rsi
	movq	%rax, (%rsi)
	movq	(%rcx), %rax
.LBB0_8:
	movq	%rax, schedulable_threads(%rip)
	xorl	%esi, %esi
	movq	%rsp, %rax
	lock		cmpxchgq	%rsi, schedulable_threads_lock(%rip)
	jne	.LBB0_9
	movq	%cr3, %rsi
	movq	32(%rcx), %rax
	cmpq	%rax, %rsi
	jne	.LBB0_11
.LBB0_12:
	lock		decq	(%rdx)
	jne	.LBB0_13
.LBB0_15:
	callq	abort@PLT
	.p2align	4, 0x90
.LBB0_9:
.Ltmp2:
	movq	8(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp3
	pause
	jmp	.Ltmp2
.Ltmp3:
	movl	$1, (%rax)
	movq	%cr3, %rsi
	movq	32(%rcx), %rax
	cmpq	%rax, %rsi
	je	.LBB0_12
.LBB0_11:
	movq	%rax, %cr3
	lock		decq	(%rdx)
	je	.LBB0_15
.LBB0_13:
	movq	%rcx, %gs:0
	leaq	2097152(%rcx), %rax
	movq	%rax, %gs:36
	movq	16(%rcx), %rsp
	jmpq	*24(%rcx)
.LBB0_14:
	lock		incq	idle_cores_num(%rip)
	sti
	addq	$16, %rsp
	jmp	empty_loop
.Lfunc_end0:
	.size	find_hook, .Lfunc_end0-find_hook
