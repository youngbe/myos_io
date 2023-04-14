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
	#APP
	movq   %gs:0, %r9
	#NO_APP
	movq	(%rsi), %rcx
	movl	$2, %eax
	cmpq	%rcx, %r9
	je	.LBB0_1
# %bb.39:
	retq
.LBB0_1:
	movq	8(%rsi), %r8
	xorl	%eax, %eax
	cmpq	$2, %r8
	cmovbq	%rsi, %rax
	movq	%rax, 64(%r9)
	movq	40(%r9), %rax
	movq	%rax, %rcx
	andq	$-2, %rcx
	testb	$1, %al
	jne	.LBB0_3
# %bb.2:
	lock		incq	(%rcx)
.LBB0_3:
	#MEMBARRIER
	#APP
	pushq      %rbp
	pushq      %r15
	pushq      %r14
	pushq      %r13
	pushq      %r12
	pushq      %rbx
	subq       $16, %rsp
	fstcw      8(%rsp)
	stmxcsr    (%rsp)
	rdfsbase   %rax
	pushq      %rax
	pushfq
	subq       $24, %rsp
	#NO_APP
	#APP
	movq   %rsp, %rdx
	#NO_APP
	movl	$0, (%rdx)
	movq	$0, 8(%rdx)
	#MEMBARRIER
	#APP
	cli
	swapgs
	rdgsbase   %rax
	swapgs
	movq       %rax, 16(%rsp)
	#NO_APP
	#MEMBARRIER
	leaq	16(%rdx), %rax
	movq	%rax, 16(%r9)
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%r9)
	#MEMBARRIER
	movq	%rdx, %rax
	xchgq	%rax, 8(%rdi)
	testq	%rax, %rax
	je	.LBB0_4
# %bb.5:
	movq	%rdx, 8(%rax)
	#MEMBARRIER
	movl	(%rdx), %eax
	testl	%eax, %eax
	jne	.LBB0_8
	.p2align	4, 0x90
.LBB0_6:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	(%rdx), %eax
	testl	%eax, %eax
	je	.LBB0_6
.LBB0_8:
	movq	(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_10
.LBB0_9:
	movq	8(%rax), %r10
	movq	%r10, 8(%r9)
	movq	8(%rax), %r10
	movq	%r9, (%r10)
	jmp	.LBB0_11
.LBB0_4:
	#MEMBARRIER
	movq	(%rdi), %rax
	testq	%rax, %rax
	jne	.LBB0_9
.LBB0_10:
	movq	%r9, %rax
.LBB0_11:
	movq	%r9, 8(%rax)
	movq	$0, (%r9)
	movq	%rax, (%rdi)
	xorl	%r9d, %r9d
	movq	%rdx, %rax
	lock		cmpxchgq	%r9, 8(%rdi)
	je	.LBB0_15
	.p2align	4, 0x90
# %bb.13:
	movq	8(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB0_14
.LBB0_12:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_12
.LBB0_14:
	movl	$1, (%rax)
.LBB0_15:
	#MEMBARRIER
	#APP
	rdgsbase   %rsp
	addq       $65520, %rsp
	#NO_APP
	cmpq	$2, %r8
	jae	.LBB0_16
# %bb.18:
	#APP
	movq     %rsp, %rdx
	#NO_APP
	movl	$0, (%rdx)
	movq	$0, 8(%rdx)
	#MEMBARRIER
	movq	%rdx, %rax
	xchgq	%rax, 24(%rsi)
	testq	%rax, %rax
	je	.LBB0_19
# %bb.20:
	movq	%rdx, 8(%rax)
	#MEMBARRIER
	movl	(%rdx), %eax
	testl	%eax, %eax
	jne	.LBB0_23
	.p2align	4, 0x90
.LBB0_21:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	(%rdx), %eax
	testl	%eax, %eax
	je	.LBB0_21
.LBB0_23:
	movq	16(%rsi), %rdi
	testq	%rdi, %rdi
	je	.LBB0_24
.LBB0_25:
	movq	8(%rdi), %r8
	cmpq	%rdi, %r8
	je	.LBB0_26
# %bb.27:
	movq	(%rdi), %rax
	movq	%r8, 8(%rax)
	jmp	.LBB0_28
.LBB0_16:
	movq	%rcx, %rdi
	#APP
	jmp    switch_to_empty
	#NO_APP
# %bb.17:
.LBB0_19:
	#MEMBARRIER
	movq	16(%rsi), %rdi
	testq	%rdi, %rdi
	jne	.LBB0_25
.LBB0_24:
	movq	$0, (%rsi)
	jmp	.LBB0_29
.LBB0_26:
	xorl	%eax, %eax
.LBB0_28:
	movq	%rax, 16(%rsi)
.LBB0_29:
	xorl	%r8d, %r8d
	movq	%rdx, %rax
	lock		cmpxchgq	%r8, 24(%rsi)
	je	.LBB0_33
	.p2align	4, 0x90
# %bb.31:
	movq	8(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB0_32
.LBB0_30:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_30
.LBB0_32:
	movl	$1, (%rax)
.LBB0_33:
	#MEMBARRIER
	testq	%rdi, %rdi
	je	.LBB0_36
# %bb.34:
	movq	%rdi, (%rsi)
	movq	%rcx, %rsi
	#APP
	jmp switch_to
	#NO_APP
# %bb.35:
.LBB0_36:
	movq	%rcx, %rdi
	#APP
	jmp    switch_to_empty
	#NO_APP
# %bb.37:
.Ltmp0:                                 # Block address taken
.LBB0_38:                               # Label of block must be emitted
	#APP
	popq       %r12
	swapgs
	wrgsbase   %r12
	swapgs
	popfq
	popq       %r13
	wrfsbase   %r13
	ldmxcsr    (%rsp)
	fldcw      8(%rsp)
	addq       $16, %rsp
	popq       %rbx
	popq       %r12
	popq       %r13
	popq       %r14
	popq       %r15
	popq       %rbp
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
