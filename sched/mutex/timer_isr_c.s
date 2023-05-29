	.text
	.file	"timer_isr_c.c"
	.section	.text.kkk,"ax",@progbits
	.globl	kkk                             # -- Begin function kkk
	.p2align	4, 0x90
	.type	kkk,@function
kkk:                                    # @kkk
.Lkkk$local:
	.type	.Lkkk$local,@function
# %bb.0:
	#APP
	nop
	#NO_APP
	#APP
	movq	%rsp, 16(%rcx)
	leaq	.Lreturn(%rip), %rax
	#NO_APP
	movq	%rax, 24(%rcx)
	movl	$0, -24(%rsp)
	movq	$0, -16(%rsp)
	movq	$0, (%rcx)
	movq	40(%rcx), %rdx
	movq	%rdx, %rsi
	andq	$-2, %rsi
	testb	$1, %dl
	jne	.LBB0_2
# %bb.1:
	lock		incq	(%rsi)
.LBB0_2:
	movq	%rcx, %rax
	xchgq	%rax, schedulable_threads+8(%rip)
	leaq	schedulable_threads(%rip), %rdi
	testq	%rax, %rax
	cmovneq	%rax, %rdi
	movq	%rcx, (%rdi)
	leaq	-24(%rsp), %r8
	movq	%r8, %rax
	xchgq	%rax, schedulable_threads_lock(%rip)
	testq	%rax, %rax
	je	.LBB0_6
# %bb.3:
	movq	%r8, 8(%rax)
	cmpl	$0, -24(%rsp)
	jne	.LBB0_6
	.p2align	4, 0x90
.LBB0_4:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	cmpl	$0, -24(%rsp)
	je	.LBB0_4
.LBB0_6:
	movq	schedulable_threads+8(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB0_7
	.p2align	4, 0x90
# %bb.9:
	movq	schedulable_threads(%rip), %r9
	testq	%r9, %r9
	jne	.LBB0_10
.LBB0_8:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	schedulable_threads(%rip), %r9
	testq	%r9, %r9
	je	.LBB0_8
.LBB0_10:
	cmpq	%rdi, %r9
	jne	.LBB0_13
# %bb.11:
	movq	$0, schedulable_threads(%rip)
	xorl	%r10d, %r10d
	movq	%rdi, %rax
	lock		cmpxchgq	%r10, schedulable_threads+8(%rip)
	je	.LBB0_15
	.p2align	4, 0x90
.LBB0_13:
	movq	(%r9), %rax
	testq	%rax, %rax
	jne	.LBB0_14
.LBB0_12:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	(%r9), %rax
	testq	%rax, %rax
	je	.LBB0_12
.LBB0_14:
	movq	%rax, schedulable_threads(%rip)
	movq	%r9, %rdi
.LBB0_15:
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%r8, %rax
	jne	.LBB0_18
	jmp	.LBB0_16
.LBB0_7:
	xorl	%edi, %edi
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%r8, %rax
	jne	.LBB0_18
.LBB0_16:
	xorl	%r8d, %r8d
	leaq	-24(%rsp), %rax
	lock		cmpxchgq	%r8, schedulable_threads_lock(%rip)
	je	.LBB0_20
	.p2align	4, 0x90
.LBB0_18:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.LBB0_19
.LBB0_17:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	je	.LBB0_17
.LBB0_19:
	movl	$1, (%rax)
.LBB0_20:
	cmpq	%rcx, %rdi
	je	.LBB0_21
# %bb.24:
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmp	switch_to_interrupt
	#NO_APP
.LBB0_21:
	testb	$1, %dl
	jne	.LBB0_23
# %bb.22:
	lock		decq	(%rsi)
.LBB0_23:
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	popq	%rsp
	addq	$72, %rsp
	wrmsr
	#NO_APP
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmp	.Lpop8_iretq
	#NO_APP
.Lfunc_end0:
	.size	kkk, .Lfunc_end0-kkk
	.size	.Lkkk$local, .Lfunc_end0-kkk
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
