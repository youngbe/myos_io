	.text
	.file	"mtx_unlock_c.c"
	.section	.text.mtx_unlock,"ax",@progbits
	.globl	mtx_unlock                      # -- Begin function mtx_unlock
	.p2align	4, 0x90
	.type	mtx_unlock,@function
mtx_unlock:                             # @mtx_unlock
.Lmtx_unlock$local:
	.type	.Lmtx_unlock$local,@function
# %bb.0:
	pushq	%rax
	#APP
	movq	%gs:0, %rax
	#NO_APP
	movl	$2, %edx
	cmpq	(%rdi), %rax
	je	.LBB0_1
.LBB0_25:
	movl	%edx, %eax
	popq	%rcx
	retq
.LBB0_1:
	movq	24(%rdi), %rax
	cmpq	$2, %rax
	jb	.LBB0_3
# %bb.2:
	decq	%rax
	movq	%rax, 24(%rdi)
	xorl	%edx, %edx
	movl	%edx, %eax
	popq	%rcx
	retq
.LBB0_3:
	leaq	8(%rdi), %rcx
	movq	8(%rdi), %rdx
	testq	%rdx, %rdx
	je	.LBB0_4
.LBB0_10:
	movq	(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB0_19
# %bb.11:
	movq	16(%rdi), %rax
	cmpq	%rdx, %rax
	jne	.LBB0_13
# %bb.12:
	movq	$0, 8(%rdi)
	movq	%rdx, %rax
	lock		cmpxchgq	%rcx, 16(%rdi)
	jne	.LBB0_13
	jmp	.LBB0_20
	.p2align	4, 0x90
.LBB0_14:                               #   in Loop: Header=BB0_13 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_13:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_14
	jmp	.LBB0_19
.LBB0_4:
	movq	16(%rdi), %rsi
	cmpq	%rcx, %rsi
	jne	.LBB0_8
# %bb.5:
	movq	$0, (%rdi)
	xorl	%edx, %edx
	movq	%rcx, %rax
	lock		cmpxchgq	%rdx, 16(%rdi)
	je	.LBB0_25
# %bb.6:
	movq	%rax, %rsi
	jmp	.LBB0_8
	.p2align	4, 0x90
.LBB0_7:                                #   in Loop: Header=BB0_8 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_8:                                # =>This Inner Loop Header: Depth=1
	movq	(%rcx), %rdx
	testq	%rdx, %rdx
	je	.LBB0_7
# %bb.9:
	testq	%rsi, %rsi
	je	.LBB0_10
# %bb.15:
	cmpq	%rdx, %rsi
	jne	.LBB0_18
# %bb.16:
	movq	$0, 8(%rdi)
	movq	%rsi, %rax
	lock		cmpxchgq	%rcx, 16(%rdi)
	jne	.LBB0_18
	jmp	.LBB0_21
	.p2align	4, 0x90
.LBB0_17:                               #   in Loop: Header=BB0_18 Depth=1
	#APP
	pause
	#NO_APP
.LBB0_18:                               # =>This Inner Loop Header: Depth=1
	movq	(%rdx), %rax
	testq	%rax, %rax
	je	.LBB0_17
.LBB0_19:
	movq	%rax, (%rcx)
.LBB0_20:
	movq	%rdx, %rsi
.LBB0_21:
	addq	$-8, %rsi
	movq	%rsi, (%rdi)
	#APP
	pushfq
	popq	%rax
	#NO_APP
	movq	%rsi, %rdi
	testl	$512, %eax                      # imm = 0x200
	jne	.LBB0_22
# %bb.23:
	callq	set_thread_schedulable@PLT
	xorl	%edx, %edx
	movl	%edx, %eax
	popq	%rcx
	retq
.LBB0_22:
	callq	cli_set_thread_schedulable@PLT
	xorl	%edx, %edx
	movl	%edx, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	mtx_unlock, .Lfunc_end0-mtx_unlock
	.size	.Lmtx_unlock$local, .Lfunc_end0-mtx_unlock
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
