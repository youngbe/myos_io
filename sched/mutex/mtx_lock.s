	.section	.text.mtx_lock,"ax",@progbits
	.globl	mtx_lock                        # -- Begin function mtx_lock
	.p2align	4, 0x90
	.type	mtx_lock,@function
mtx_lock:                               # @mtx_lock
	movq	%gs:0, %rsi
	xorl	%ecx, %ecx
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, (%rdi)
	je	.LBB0_23
	cmpq	%rax, %rsi
	je	.LBB0_2
	movq	48(%rsi), %rdx
	lock		incq	(%rdx)
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$16, %rsp
	fstcw	8(%rsp)
	stmxcsr	(%rsp)
	rdfsbaseq	%r9
	pushq	%r9
	pushfq
	movq	(%rsp), %r8
	leaq	-8(%rsp), %rax
	subq	$24, %rsp
	movq	%rsp, %rcx
	movl	$0, (%rcx)
	movq	$0, 8(%rcx)
	testl	$512, %r8d                      # imm = 0x200
	je	.LBB0_6
	cli
.LBB0_6:
	swapgs
	rdgsbaseq	%r9
	swapgs
	movq	%r9, 16(%rsp)
	movq	%rax, 16(%rsi)
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%rsi)
	movq	%rdx, %gs:8
	movq	$0, %gs:0
	movq	%rcx, %rax
	xchgq	%rax, 24(%rdi)
	testq	%rax, %rax
	je	.LBB0_8
	movq	%rcx, 8(%rax)
	xorl	%eax, %eax
	.p2align	4, 0x90
.Ltmp1:
	cmpl	(%rcx), %eax
	jne	.Ltmp2
	pause
	jmp	.Ltmp1
.Ltmp2:
.LBB0_8:
	lock		cmpxchgq	%rsi, (%rdi)
	jne	.LBB0_14
	xorl	%r9d, %r9d
	movq	%rcx, %rax
	lock		cmpxchgq	%r9, 24(%rdi)
	je	.LBB0_11
	.p2align	4, 0x90
.Ltmp3:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.Ltmp4
	pause
	jmp	.Ltmp3
.Ltmp4:
	movl	$1, (%rax)
.LBB0_11:
	movq	%rsi, %gs:0
	testl	$512, %r8d                      # imm = 0x200
	je	.LBB0_13
	sti
.LBB0_13:
	lock		decq	(%rdx)
	addq	$104, %rsp
	xorl	%eax, %eax
	retq
.LBB0_2:
	movq	8(%rdi), %rax
	movl	$2, %ecx
	incq	%rax
	cmpq	$2, %rax
	jae	.LBB0_3
.LBB0_23:
	movl	%ecx, %eax
	retq
.LBB0_14:
	movq	16(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_16
	movq	8(%rax), %r8
	movq	%r8, 8(%rsi)
	movq	%rax, (%rsi)
	movq	%rsi, 8(%rax)
	movq	8(%rsi), %rax
	movq	%rsi, (%rax)
	xorl	%esi, %esi
	movq	%rcx, %rax
	lock		cmpxchgq	%rsi, 24(%rdi)
	je	.LBB0_19
	.p2align	4, 0x90
.LBB0_18:
.Ltmp5:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.Ltmp6
	pause
	jmp	.Ltmp5
.Ltmp6:
	movl	$1, (%rax)
.LBB0_19:
	lock		incq	idle_cores_num(%rip)
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	sti
	jmp	find_hook
.LBB0_3:
	movq	%rax, 8(%rdi)
	xorl	%eax, %eax
	retq
.LBB0_16:
	movq	%rsi, 8(%rsi)
	movq	%rsi, (%rsi)
	movq	%rsi, 16(%rdi)
	xorl	%esi, %esi
	movq	%rcx, %rax
	lock		cmpxchgq	%rsi, 24(%rdi)
	jne	.LBB0_18
	jmp	.LBB0_19
.Ltmp0:                                 # Block address taken
	popq	%r12
	swapgs
	wrgsbaseq	%r12
	swapgs
	popfq
	popq	%r13
	wrfsbaseq	%r13
	ldmxcsr	(%rsp)
	fldcw	8(%rsp)
	addq	$16, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	xorl	%eax, %eax
	retq
.Lfunc_end0:
	.size	mtx_lock, .Lfunc_end0-mtx_lock
