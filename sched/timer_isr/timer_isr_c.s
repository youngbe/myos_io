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
	movq	40(%rcx), %rdx
	movq	%rdx, %rsi
	andq	$-2, %rsi
	testb	$1, %dl
	jne	.LBB0_2
# %bb.1:
	lock		incq	(%rsi)
.LBB0_2:
	movl	$0, -24(%rsp)
	movq	$0, -16(%rsp)
	#MEMBARRIER
	leaq	-24(%rsp), %rax
	movq	%rax, %rdi
	xchgq	%rdi, schedulable_threads_lock(%rip)
	testq	%rdi, %rdi
	je	.LBB0_3
# %bb.4:
	movq	%rax, 8(%rdi)
	#MEMBARRIER
	movl	-24(%rsp), %eax
	testl	%eax, %eax
	jne	.LBB0_7
	.p2align	4, 0x90
.LBB0_5:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	-24(%rsp), %eax
	testl	%eax, %eax
	je	.LBB0_5
	jmp	.LBB0_7
.LBB0_3:
	#MEMBARRIER
.LBB0_7:
	movq	schedulable_threads(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB0_12
# %bb.8:
	movq	8(%rdi), %rax
	movq	%rax, 8(%rcx)
	movq	8(%rdi), %rax
	movq	%rcx, (%rax)
	movq	%rcx, 8(%rdi)
	movq	$0, (%rcx)
	cmpq	%rdi, %rcx
	je	.LBB0_9
# %bb.10:
	movq	(%rdi), %rax
	movq	%rcx, 8(%rax)
	jmp	.LBB0_11
.LBB0_9:
	xorl	%eax, %eax
.LBB0_11:
	movq	%rax, schedulable_threads(%rip)
.LBB0_12:
	xorl	%ecx, %ecx
	leaq	-24(%rsp), %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	je	.LBB0_16
	.p2align	4, 0x90
# %bb.14:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.LBB0_15
.LBB0_13:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	je	.LBB0_13
.LBB0_15:
	movl	$1, (%rax)
.LBB0_16:
	#MEMBARRIER
	testq	%rdi, %rdi
	je	.LBB0_17
# %bb.20:
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmp	switch_to_interrupt
	#NO_APP
.LBB0_17:
	testb	$1, %dl
	jne	.LBB0_19
# %bb.18:
	lock		decq	(%rsi)
.LBB0_19:
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
	#MEMBARRIER
	movq	%rcx, %rax
	xchgq	%rax, schedulable_threads_lock(%rip)
	testq	%rax, %rax
	je	.LBB1_6
# %bb.7:
	movq	%rcx, 8(%rax)
	#MEMBARRIER
	movl	(%rcx), %eax
	testl	%eax, %eax
	jne	.LBB1_10
	.p2align	4, 0x90
.LBB1_8:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	(%rcx), %eax
	testl	%eax, %eax
	je	.LBB1_8
	jmp	.LBB1_10
.LBB1_6:
	#MEMBARRIER
.LBB1_10:
	movq	schedulable_threads(%rip), %rdi
	movq	8(%rdi), %rdx
	cmpq	%rdi, %rdx
	je	.LBB1_11
# %bb.12:
	movq	(%rdi), %rax
	movq	%rdx, 8(%rax)
	jmp	.LBB1_13
.LBB1_11:
	xorl	%eax, %eax
.LBB1_13:
	movq	%rax, schedulable_threads(%rip)
	xorl	%edx, %edx
	movq	%rcx, %rax
	lock		cmpxchgq	%rdx, schedulable_threads_lock(%rip)
	je	.LBB1_17
	.p2align	4, 0x90
# %bb.15:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.LBB1_16
.LBB1_14:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.LBB1_14
.LBB1_16:
	movl	$1, (%rax)
.LBB1_17:
	#MEMBARRIER
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
