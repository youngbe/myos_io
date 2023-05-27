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
	movq	%gs:0, %rcx
	#NO_APP
	movq	(%rdi), %rax
	leaq	-8(%rax), %rdx
	testq	%rax, %rax
	cmoveq	%rax, %rdx
	cmpq	%rcx, %rdx
	je	.LBB0_1
# %bb.3:
	#APP
	pushfq
	popq	%rdx
	#NO_APP
	testq	%rax, %rax
	jne	.LBB0_14
# %bb.4:
	movq	$0, 8(%rcx)
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_6
# %bb.5:
	#MEMBARRIER
	#APP
	cli
	#NO_APP
	#MEMBARRIER
.LBB0_6:
	movq	8(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_8
# %bb.7:
	xorl	%esi, %esi
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_12
	jmp	.LBB0_11
.LBB0_1:
	movq	16(%rdi), %rcx
	movl	$2, %eax
	incq	%rcx
	cmpq	$2, %rcx
	jae	.LBB0_2
# %bb.28:
	retq
.LBB0_2:
	movq	%rcx, 16(%rdi)
	#MEMBARRIER
	xorl	%eax, %eax
	retq
.LBB0_8:
	leaq	8(%rcx), %r8
	xorl	%esi, %esi
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, 8(%rdi)
	je	.LBB0_9
# %bb.10:
	testl	$512, %edx                      # imm = 0x200
	jne	.LBB0_11
.LBB0_12:
	testb	%sil, %sil
	je	.LBB0_14
.LBB0_13:
	#MEMBARRIER
	xorl	%eax, %eax
	retq
.LBB0_9:
	movq	%r8, (%rdi)
	movb	$1, %sil
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_12
.LBB0_11:
	#MEMBARRIER
	#APP
	sti
	#NO_APP
	testb	%sil, %sil
	jne	.LBB0_13
.LBB0_14:
	movq	40(%rcx), %rsi
	movq	%rsi, %rax
	andq	$-2, %rax
	testb	$1, %sil
	jne	.LBB0_16
# %bb.15:
	lock		incq	(%rax)
.LBB0_16:
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
	rdfsbaseq	%r8
	pushq	%r8
	#NO_APP
	#APP
	pushq	%rdx
	#NO_APP
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_18
# %bb.17:
	#MEMBARRIER
	#APP
	cli
	#NO_APP
	#MEMBARRIER
.LBB0_18:
	#APP
	swapgs
	rdgsbaseq	%r8
	swapgs
	pushq	%r8
	#NO_APP
	#APP
	movq	%rsp, 16(%rcx)
	#NO_APP
	leaq	.Ltmp0(%rip), %r8
	movq	%r8, 24(%rcx)
	#MEMBARRIER
	movq	%rcx, %r8
	addq	$8, %r8
	movq	$0, 8(%rcx)
	movq	%r8, %rcx
	xchgq	%rcx, 8(%rdi)
	testq	%rcx, %rcx
	cmoveq	%rdi, %rcx
	movq	%r8, (%rcx)
	je	.LBB0_19
# %bb.24:
	movq	%rax, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
	#NO_APP
# %bb.25:
.LBB0_19:
	testl	$512, %edx                      # imm = 0x200
	je	.LBB0_21
# %bb.20:
	#APP
	sti
	#NO_APP
	#MEMBARRIER
.LBB0_21:
	testb	$1, %sil
	jne	.LBB0_23
# %bb.22:
	lock		decq	(%rax)
.LBB0_23:
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
