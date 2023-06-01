	.text
	.file	"cnd_wait_c.c"
	.section	.text.cnd_wait,"ax",@progbits
	.globl	cnd_wait                        # -- Begin function cnd_wait
	.p2align	4, 0x90
	.type	cnd_wait,@function
cnd_wait:                               # @cnd_wait
.Lcnd_wait$local:
	.type	.Lcnd_wait$local,@function
# %bb.0:
	movl	$2, %eax
	cmpq	$0, (%rsi)
	je	.LBB0_34
# %bb.1:
	#APP
	movq	%gs:0, %r9
	#NO_APP
	cmpq	%r9, (%rsi)
	je	.LBB0_2
.LBB0_34:
	retq
.LBB0_2:
	movq	24(%rsi), %rax
	xorl	%ecx, %ecx
	cmpq	$2, %rax
	cmovaeq	%rcx, %rsi
	movq	%rsi, 8(%r9)
	movq	40(%r9), %rdx
	testb	$1, %dl
	jne	.LBB0_4
# %bb.3:
	lock		incq	(%rdx)
	movq	%rdx, %rcx
.LBB0_4:
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
	rdfsbaseq	%rdx
	pushq	%rdx
	#NO_APP
	#APP
	pushfq
	movq	(%rsp), %rdx
	#NO_APP
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_6
# %bb.5:
	#APP
	cli
	#NO_APP
.LBB0_6:
	#APP
	swapgs
	rdgsbaseq	%rdx
	swapgs
	pushq	%rdx
	#NO_APP
	#APP
	movq	%rsp, 16(%r9)
	#NO_APP
	leaq	.Ltmp0(%rip), %rdx
	movq	%rdx, 24(%r9)
	movq	%r9, %rdx
	xchgq	%rdx, 8(%rdi)
	testq	%rdx, %rdx
	cmoveq	%rdi, %rdx
	movq	%r9, (%rdx)
	cmpq	$2, %rax
	jb	.LBB0_11
.LBB0_7:
	testq	%rcx, %rcx
	jne	.LBB0_9
# %bb.8:
	movq	40(%r9), %rcx
	andq	$-2, %rcx
.LBB0_9:
	movq	%rcx, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
	#NO_APP
# %bb.10:
.LBB0_11:
	leaq	8(%rsi), %rdx
	movq	8(%rsi), %r8
	testq	%r8, %r8
	je	.LBB0_12
.LBB0_18:
	movq	(%r8), %rax
	testq	%rax, %rax
	jne	.LBB0_27
# %bb.19:
	movq	16(%rsi), %rax
	cmpq	%r8, %rax
	jne	.LBB0_21
# %bb.20:
	movq	$0, 8(%rsi)
	movq	%r8, %rax
	lock		cmpxchgq	%rdx, 16(%rsi)
	jne	.LBB0_21
	jmp	.LBB0_28
	.p2align	4, 0x90
.LBB0_22:                               #   in Loop: Header=BB0_21 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_21:                               # =>This Inner Loop Header: Depth=1
	movq	(%r8), %rax
	testq	%rax, %rax
	je	.LBB0_22
	jmp	.LBB0_27
.LBB0_12:
	movq	16(%rsi), %rdi
	cmpq	%rdx, %rdi
	jne	.LBB0_16
# %bb.13:
	movq	$0, (%rsi)
	xorl	%edi, %edi
	movq	%rdx, %rax
	lock		cmpxchgq	%rdi, 16(%rsi)
	je	.LBB0_7
# %bb.14:
	movq	%rax, %rdi
	jmp	.LBB0_16
	.p2align	4, 0x90
.LBB0_15:                               #   in Loop: Header=BB0_16 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_16:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdx), %r8
	testq	%r8, %r8
	je	.LBB0_15
# %bb.17:
	testq	%rdi, %rdi
	je	.LBB0_18
# %bb.23:
	cmpq	%r8, %rdi
	jne	.LBB0_26
# %bb.24:
	movq	$0, 8(%rsi)
	movq	%rdi, %rax
	lock		cmpxchgq	%rdx, 16(%rsi)
	jne	.LBB0_26
	jmp	.LBB0_29
	.p2align	4, 0x90
.LBB0_25:                               #   in Loop: Header=BB0_26 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_26:                               # =>This Inner Loop Header: Depth=1
	movq	(%r8), %rax
	testq	%rax, %rax
	je	.LBB0_25
.LBB0_27:
	movq	%rax, (%rdx)
.LBB0_28:
	movq	%r8, %rdi
.LBB0_29:
	addq	$-8, %rdi
	movq	%rdi, (%rsi)
	testq	%rcx, %rcx
	jne	.LBB0_31
# %bb.30:
	movq	40(%r9), %rcx
	andq	$-2, %rcx
.LBB0_31:
	movq	%rcx, %rsi
	#APP
	jmp	switch_to
	#NO_APP
# %bb.32:
.Ltmp0:                                 # Block address taken
.LBB0_33:                               # Label of block must be emitted
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
	.size	cnd_wait, .Lfunc_end0-cnd_wait
	.size	.Lcnd_wait$local, .Lfunc_end0-cnd_wait
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
