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
	#APP
	movq	%gs:0, %rsi
	#NO_APP
	leaq	8(%rsi), %rdx
	#APP
	#NO_APP
	movq	$0, 8(%rsi)
	#APP
	pushfq
	popq	%rcx
	#NO_APP
	testl	$512, %ecx                      # imm = 0x200
	jne	.LBB0_2
# %bb.1:
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_3
# %bb.21:
	cmpq	%rax, %rdx
	jne	.LBB0_10
	jmp	.LBB0_22
.LBB0_2:
	#APP
	cli
	#NO_APP
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_3
# %bb.20:
	#APP
	sti
	#NO_APP
	cmpq	%rax, %rdx
	jne	.LBB0_10
.LBB0_22:
	movq	16(%rdi), %rcx
	movl	$2, %eax
	incq	%rcx
	cmpq	$2, %rcx
	jae	.LBB0_23
.LBB0_28:
	retq
.LBB0_3:
	cmpq	$0, 8(%rdi)
	je	.LBB0_5
# %bb.4:
	xorl	%r8d, %r8d
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_9
	jmp	.LBB0_8
.LBB0_5:
	xorl	%r8d, %r8d
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, 8(%rdi)
	je	.LBB0_6
# %bb.7:
	testl	$512, %ecx                      # imm = 0x200
	jne	.LBB0_8
.LBB0_9:
	xorl	%eax, %eax
	testb	%r8b, %r8b
	jne	.LBB0_28
	jmp	.LBB0_10
.LBB0_23:
	movq	%rcx, 16(%rdi)
	xorl	%eax, %eax
	retq
.LBB0_6:
	movq	%rdx, (%rdi)
	movb	$1, %r8b
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_9
.LBB0_8:
	#APP
	sti
	#NO_APP
	xorl	%eax, %eax
	testb	%r8b, %r8b
	jne	.LBB0_28
.LBB0_10:
	movq	40(%rsi), %r8
	movq	%r8, %rax
	andq	$-2, %rax
	testb	$1, %r8b
	jne	.LBB0_12
# %bb.11:
	lock		incq	(%rax)
.LBB0_12:
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
	rdfsbaseq	%r9
	pushq	%r9
	#NO_APP
	#APP
	pushq	%rcx
	#NO_APP
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_14
# %bb.13:
	#APP
	cli
	#NO_APP
.LBB0_14:
	#APP
	swapgs
	rdgsbaseq	%r9
	swapgs
	pushq	%r9
	#NO_APP
	#APP
	movq	%rsp, 16(%rsi)
	#NO_APP
	leaq	.Ltmp0(%rip), %r9
	movq	%r9, 24(%rsi)
	movq	%rdx, %rsi
	xchgq	%rsi, 8(%rdi)
	testq	%rsi, %rsi
	cmoveq	%rdi, %rsi
	movq	%rdx, (%rsi)
	je	.LBB0_15
# %bb.24:
	movq	%rax, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
	#NO_APP
# %bb.25:
.LBB0_15:
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_17
# %bb.16:
	#APP
	sti
	#NO_APP
.LBB0_17:
	testb	$1, %r8b
	jne	.LBB0_19
# %bb.18:
	lock		decq	(%rax)
.LBB0_19:
	#APP
	addq	$88, %rsp
	#NO_APP
	xorl	%eax, %eax
	retq
.Ltmp0:                                 # Block address taken
.LBB0_26:                               # Label of block must be emitted
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
	.size	.Lmtx_lock$local, .Lfunc_end0-mtx_lock
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
