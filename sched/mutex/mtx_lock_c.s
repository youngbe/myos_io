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
	movq   %gs:0, %r8
	#NO_APP
	xorl	%edx, %edx
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, (%rdi)
	je	.LBB0_32
# %bb.1:
	movq	%rdi, %rcx
	cmpq	%rax, %r8
	je	.LBB0_2
# %bb.4:
	movq	40(%r8), %rsi
	movq	%rsi, %rdi
	andq	$-2, %rdi
	testb	$1, %sil
	jne	.LBB0_6
# %bb.5:
	lock		incq	(%rdi)
.LBB0_6:
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
	rdfsbase   %r9
	pushq      %r9
	pushfq
	movq       (%rsp), %r9
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
	movq	%rax, 16(%r8)
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%r8)
	#MEMBARRIER
	movq	%rdx, %rax
	xchgq	%rax, 24(%rcx)
	testq	%rax, %rax
	je	.LBB0_7
# %bb.8:
	movq	%rdx, 8(%rax)
	#MEMBARRIER
	movl	(%rdx), %eax
	testl	%eax, %eax
	jne	.LBB0_11
	.p2align	4, 0x90
.LBB0_9:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	(%rdx), %eax
	testl	%eax, %eax
	je	.LBB0_9
.LBB0_11:
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, (%rcx)
	jne	.LBB0_12
.LBB0_20:
	xorl	%r8d, %r8d
	movq	%rdx, %rax
	lock		cmpxchgq	%r8, 24(%rcx)
	jne	.LBB0_22
# %bb.24:
	#MEMBARRIER
	testl	$512, %r9d                      # imm = 0x200
	jne	.LBB0_25
.LBB0_26:
	#MEMBARRIER
	testb	$1, %sil
	jne	.LBB0_28
.LBB0_27:
	lock		decq	(%rdi)
.LBB0_28:
	#APP
	addq  $104, %rsp
	#NO_APP
	xorl	%edx, %edx
	movl	%edx, %eax
	retq
	.p2align	4, 0x90
.LBB0_22:                               # =>This Inner Loop Header: Depth=1
	movq	8(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB0_23
# %bb.21:                               #   in Loop: Header=BB0_22 Depth=1
	#APP
	pause
	#NO_APP
	jmp	.LBB0_22
.LBB0_2:
	movq	8(%rcx), %rax
	movl	$2, %edx
	incq	%rax
	cmpq	$2, %rax
	jae	.LBB0_3
.LBB0_32:
	movl	%edx, %eax
	retq
.LBB0_7:
	#MEMBARRIER
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, (%rcx)
	je	.LBB0_20
.LBB0_12:
	movq	16(%rcx), %rax
	testq	%rax, %rax
	je	.LBB0_14
# %bb.13:
	movq	8(%rax), %rsi
	movq	%rsi, 8(%r8)
	movq	8(%rax), %rsi
	movq	%r8, (%rsi)
	jmp	.LBB0_15
.LBB0_23:
	movl	$1, (%rax)
	#MEMBARRIER
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_26
.LBB0_25:
	#APP
	sti
	#NO_APP
	#MEMBARRIER
	testb	$1, %sil
	je	.LBB0_27
	jmp	.LBB0_28
.LBB0_3:
	movq	%rax, 8(%rcx)
	#MEMBARRIER
	xorl	%edx, %edx
	movl	%edx, %eax
	retq
.LBB0_14:
	movq	%r8, %rax
.LBB0_15:
	movq	%r8, 8(%rax)
	movq	$0, (%r8)
	movq	%rax, 16(%rcx)
	xorl	%esi, %esi
	movq	%rdx, %rax
	lock		cmpxchgq	%rsi, 24(%rcx)
	je	.LBB0_19
	.p2align	4, 0x90
# %bb.17:
	movq	8(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB0_18
.LBB0_16:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_16
.LBB0_18:
	movl	$1, (%rax)
.LBB0_19:
	#MEMBARRIER
	#APP
	rdgsbase   %rsp
	addq       $65520, %rsp
	jmp        switch_to_empty
	#NO_APP
# %bb.29:
.Ltmp0:                                 # Block address taken
.LBB0_30:                               # Label of block must be emitted
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
