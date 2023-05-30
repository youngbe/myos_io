	.section	.text.mtx_lock,"ax",@progbits
	.globl	mtx_lock                        # -- Begin function mtx_lock
	.p2align	4, 0x90
	.type	mtx_lock,@function
mtx_lock:                               # @mtx_lock
	movq	%gs:0, %rdx
	#NO_APP
	cmpq	%rdx, (%rdi)
	je	.LBB0_4
# %bb.1:
	movq	%rdx, %rcx
	leaq	8(%rdi), %r9
	#APP
	#NO_APP
	leaq	16(%rdi), %r8
	#APP
	#NO_APP
	#APP
	pushfq
	popq	%rsi
	#NO_APP
	testl	$512, %esi                      # imm = 0x200
	jne	.LBB0_6
# %bb.2:
	cmpq	$0, (%r8)
	jne	.LBB0_10
# %bb.3:
	xorl	%eax, %eax
	lock		cmpxchgq	%r9, (%r8)
	jne	.LBB0_10
.LBB0_21:
	movq	%rcx, (%rdi)
	xorl	%eax, %eax
	retq
.LBB0_4:
	movq	24(%rdi), %rcx
	movl	$2, %eax
	incq	%rcx
	cmpq	$2, %rcx
	jae	.LBB0_8
# %bb.5:
	retq
.LBB0_6:
	#APP
	cli
	#NO_APP
	cmpq	$0, (%r8)
	je	.LBB0_9
# %bb.7:
	#APP
	sti
	#NO_APP
	jmp	.LBB0_10
.LBB0_8:
	movq	%rcx, 24(%rdi)
	xorl	%eax, %eax
	retq
.LBB0_9:
	xorl	%eax, %eax
	lock		cmpxchgq	%r9, (%r8)
	sti
	je	.LBB0_21
.LBB0_10:
	movq	$0, 8(%rcx)
	movq	40(%rcx), %r9
	movq	%r9, %rax
	andq	$-2, %rax
	testb	$1, %r9b
	jne	.LBB0_12
# %bb.11:
	lock		incq	(%rax)
.LBB0_12:
	addq	$8, %rdx
	#APP
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$16, %rsp
	wait
	fnstcw	8(%rsp)
	stmxcsr	(%rsp)
	rdfsbaseq	%r10
	pushq	%r10
	#NO_APP
	#APP
	pushq	%rsi
	#NO_APP
	testl	$512, %esi                      # imm = 0x200
	je	.LBB0_14
# %bb.13:
	#APP
	cli
	#NO_APP
.LBB0_14:
	#APP
	swapgs
	rdgsbaseq	%r10
	swapgs
	pushq	%r10
	#NO_APP
	#APP
	movq	%rsp, 16(%rcx)
	#NO_APP
	leaq	.Ltmp0(%rip), %r10
	movq	%r10, 24(%rcx)
	movq	%rdx, %r10
	xchgq	%r10, (%r8)
	testq	%r10, %r10
	je	.LBB0_17
# %bb.15:
	movq	%rdx, (%r10)
	movq	%rax, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
	#NO_APP
# %bb.16:
.LBB0_17:
	testl	$512, %esi                      # imm = 0x200
	je	.LBB0_19
# %bb.18:
	#APP
	sti
	#NO_APP
.LBB0_19:
	#APP
	addq	$88, %rsp
	#NO_APP
	testb	$1, %r9b
	jne	.LBB0_21
# %bb.20:
	lock		decq	(%rax)
	movq	%rcx, (%rdi)
	xorl	%eax, %eax
	retq
.Ltmp0:                                 # Block address taken
.LBB0_22:                               # Label of block must be emitted
	#APP
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
	#NO_APP
	xorl	%eax, %eax
	vzeroupper
	retq
.Lfunc_end0:
	.size	mtx_lock, .Lfunc_end0-mtx_lock
