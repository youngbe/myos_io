	.text
	.file	"mtx_lock_c.c"
	.section	.text.mtx_lock,"ax",@progbits
	.globl	mtx_lock                        # -- Begin function mtx_lock
	.p2align	4, 0x90
	.type	mtx_lock,@function
mtx_lock:                               # @mtx_lock
.Lmtx_lock$local:
	.type	.Lmtx_lock$local,@function
# %bb.0:
	leaq	8(%rdi), %rsi
	#APP
	#NO_APP
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_1
# %bb.7:
	#APP
	movq	%gs:0, %r8
	#NO_APP
	cmpq	%r8, %rax
	je	.LBB0_8
# %bb.4:
	movq	$0, 8(%r8)
	movq	40(%r8), %rcx
	testb	$1, %cl
	jne	.LBB0_5
.LBB0_10:
	lock		incq	(%rcx)
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
	rdfsbaseq	%rax
	pushq	%rax
	#NO_APP
	#APP
	pushfq
	movq	(%rsp), %r9
	#NO_APP
	testl	$512, %r9d                      # imm = 0x200
	jne	.LBB0_12
	jmp	.LBB0_13
.LBB0_1:
	cmpq	$0, 16(%rdi)
	jne	.LBB0_3
# %bb.2:
	xorl	%edx, %edx
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 16(%rdi)
	jne	.LBB0_3
# %bb.6:
	#APP
	movq	%gs:0, %rax
	#NO_APP
	movq	%rax, (%rdi)
	movl	%edx, %eax
	retq
.LBB0_3:
	#APP
	movq	%gs:0, %r8
	#NO_APP
	movq	$0, 8(%r8)
	movq	40(%r8), %rcx
	testb	$1, %cl
	je	.LBB0_10
.LBB0_5:
	xorl	%ecx, %ecx
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
	rdfsbaseq	%rax
	pushq	%rax
	#NO_APP
	#APP
	pushfq
	movq	(%rsp), %r9
	#NO_APP
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_13
.LBB0_12:
	#APP
	cli
	#NO_APP
.LBB0_13:
	#APP
	swapgs
	rdgsbaseq	%rax
	swapgs
	pushq	%rax
	#NO_APP
	#APP
	movq	%rsp, 16(%r8)
	#NO_APP
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%r8)
	cmpq	$0, 16(%rdi)
	jne	.LBB0_19
# %bb.14:
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 16(%rdi)
	je	.LBB0_15
.LBB0_19:
	leaq	8(%r8), %rdx
	movq	%rdx, %rax
	xchgq	%rax, 16(%rdi)
	testq	%rax, %rax
	je	.LBB0_20
# %bb.28:
	movq	%rdx, (%rax)
	testq	%rcx, %rcx
	jne	.LBB0_30
# %bb.29:
	movq	40(%r8), %rcx
	andq	$-2, %rcx
.LBB0_30:
	movq	%rcx, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
	#NO_APP
# %bb.31:
.LBB0_8:
	movq	24(%rdi), %rax
	movl	$2, %edx
	incq	%rax
	cmpq	$2, %rax
	jae	.LBB0_9
.LBB0_34:
	movl	%edx, %eax
	retq
.LBB0_20:
	movq	%rdx, %rax
	lock		cmpxchgq	%rsi, 16(%rdi)
	jne	.LBB0_21
.LBB0_15:
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_17
# %bb.16:
	#APP
	sti
	#NO_APP
.LBB0_17:
	movq	%r8, (%rdi)
	#APP
	addq	$88, %rsp
	#NO_APP
	xorl	%edx, %edx
	testq	%rcx, %rcx
	je	.LBB0_34
# %bb.18:
	lock		decq	(%rcx)
	movl	%edx, %eax
	retq
.LBB0_21:
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_23
# %bb.22:
	#APP
	sti
	#NO_APP
.LBB0_23:
	movq	%r8, (%rdi)
	#APP
	addq	$88, %rsp
	#NO_APP
	testq	%rcx, %rcx
	je	.LBB0_26
# %bb.24:
	lock		decq	(%rcx)
	jmp	.LBB0_26
	.p2align	4, 0x90
.LBB0_25:                               #   in Loop: Header=BB0_26 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_26:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_25
# %bb.27:
	movq	%rax, (%rsi)
	xorl	%edx, %edx
	movl	%edx, %eax
	retq
.LBB0_9:
	movq	%rax, 24(%rdi)
	xorl	%edx, %edx
	movl	%edx, %eax
	retq
.Ltmp0:                                 # Block address taken
.LBB0_32:                               # Label of block must be emitted
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
	xorl	%edx, %edx
	movl	%edx, %eax
	vzeroupper
	retq
.Lfunc_end0:
	.size	mtx_lock, .Lfunc_end0-mtx_lock
	.size	.Lmtx_lock$local, .Lfunc_end0-mtx_lock
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
