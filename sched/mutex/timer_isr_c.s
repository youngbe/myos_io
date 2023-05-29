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
	movq	schedulable_threads(%rip), %rax
	movq	$0, (%rax)
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
	movq	schedulable_threads(%rip), %rdi
	movq	%rax, (%rdi)
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
	addq	$96, %rsp
	wrmsr
	#NO_APP
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmp	.Lpop5_iretq
	#NO_APP
.Lfunc_end0:
	.size	kkk, .Lfunc_end0-kkk
	.size	.Lkkk$local, .Lfunc_end0-kkk
                                        # -- End function
	.section	.text.kkk2,"ax",@progbits
	.globl	kkk2                            # -- Begin function kkk2
	.p2align	4, 0x90
	.type	kkk2,@function
kkk2:                                   # @kkk2
.Lkkk2$local:
	.type	.Lkkk2$local,@function
# %bb.0:
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	testq	%rax, %rax
	je	.LBB1_4
	.p2align	4, 0x90
.LBB1_2:                                # =>This Inner Loop Header: Depth=1
	leaq	-1(%rax), %rcx
	lock		cmpxchgq	%rcx, schedulable_threads_num(%rip)
	je	.LBB1_5
# %bb.3:                                #   in Loop: Header=BB1_2 Depth=1
	#APP
	pause
	#NO_APP
	testq	%rax, %rax
	jne	.LBB1_2
.LBB1_4:
	lock		incq	idle_cores_num(%rip)
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	rdgsbaseq	%rsp
	addq	$65536, %rsp                    # imm = 0x10000
	wrmsr
	sti
	jmp	empty_loop
	#NO_APP
.LBB1_5:
	#APP
	movq	%rsp, %rcx
	#NO_APP
	movl	$0, (%rcx)
	movq	$0, 8(%rcx)
	movq	%rcx, %rax
	xchgq	%rax, schedulable_threads_lock(%rip)
	testq	%rax, %rax
	je	.LBB1_9
# %bb.6:
	movq	%rcx, 8(%rax)
	cmpl	$0, (%rcx)
	jne	.LBB1_9
	.p2align	4, 0x90
.LBB1_7:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	cmpl	$0, (%rcx)
	je	.LBB1_7
.LBB1_9:
	movq	schedulable_threads+8(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB1_10
	.p2align	4, 0x90
# %bb.12:
	movq	schedulable_threads(%rip), %rdx
	testq	%rdx, %rdx
	jne	.LBB1_13
.LBB1_11:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	schedulable_threads(%rip), %rdx
	testq	%rdx, %rdx
	je	.LBB1_11
.LBB1_13:
	cmpq	%rdi, %rdx
	jne	.LBB1_16
# %bb.14:
	movq	schedulable_threads(%rip), %rax
	movq	$0, (%rax)
	xorl	%esi, %esi
	movq	%rdi, %rax
	lock		cmpxchgq	%rsi, schedulable_threads+8(%rip)
	je	.LBB1_18
	.p2align	4, 0x90
.LBB1_16:
	movq	(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB1_17
.LBB1_15:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	(%rdx), %rax
	testq	%rax, %rax
	je	.LBB1_15
.LBB1_17:
	movq	schedulable_threads(%rip), %rsi
	movq	%rax, (%rsi)
	movq	%rdx, %rdi
.LBB1_18:
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%rcx, %rax
	jne	.LBB1_21
	jmp	.LBB1_19
.LBB1_10:
	xorl	%edi, %edi
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%rcx, %rax
	jne	.LBB1_21
.LBB1_19:
	xorl	%edx, %edx
	movq	%rcx, %rax
	lock		cmpxchgq	%rdx, schedulable_threads_lock(%rip)
	je	.LBB1_23
	.p2align	4, 0x90
.LBB1_21:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.LBB1_22
.LBB1_20:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.LBB1_20
.LBB1_22:
	movl	$1, (%rax)
.LBB1_23:
	movq	kernel_gs_base+8(%rip), %rsi
	#APP
	jmp	switch_to_interrupt
	#NO_APP
.Lfunc_end1:
	.size	kkk2, .Lfunc_end1-kkk2
	.size	.Lkkk2$local, .Lfunc_end1-kkk2
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
