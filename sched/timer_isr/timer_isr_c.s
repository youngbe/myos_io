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
	#NO_APP
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%rcx)
	movl	$0, -24(%rsp)
	movq	$0, -16(%rsp)
	leaq	-24(%rsp), %rax
	movq	%rax, %rdx
	xchgq	%rdx, schedulable_threads_lock(%rip)
	testq	%rdx, %rdx
	je	.LBB0_2
# %bb.1:
	movq	%rax, 8(%rdx)
	#MEMBARRIER
	xorl	%eax, %eax
	#APP
	.p2align	4, 0x90
.Ltmp1:
	cmpl	-24(%rsp), %eax
	jne	.Ltmp2
	pause
	jmp	.Ltmp1
.Ltmp2:
	#NO_APP
.LBB0_2:
	#MEMBARRIER
	movq	schedulable_threads(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB0_3
# %bb.6:
	movq	(%rdi), %rax
	cmpq	%rdi, %rax
	je	.LBB0_7
# %bb.8:
	movq	%rax, (%rcx)
	movq	8(%rdi), %rdx
	movq	%rdx, 8(%rcx)
	movq	%rcx, 8(%rax)
	movq	8(%rcx), %rax
	movq	%rcx, (%rax)
	movq	(%rcx), %rcx
	jmp	.LBB0_9
.LBB0_3:
	xorl	%ecx, %ecx
	leaq	-24(%rsp), %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	je	.LBB0_5
# %bb.4:
	#APP
	.p2align	4, 0x90
.Ltmp3:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp4
	pause
	jmp	.Ltmp3
.Ltmp4:
	#NO_APP
	movl	$1, (%rax)
.LBB0_5:
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	popq	%rsp
	addq	$104, %rsp
	wrmsr
	lock		incq	old_schedulable_threads_num(%rip)
	jmp	.Lpop4_iretq
	#NO_APP
.LBB0_7:
	movq	%rcx, (%rcx)
	movq	%rcx, 8(%rcx)
.LBB0_9:
	movq	%rcx, schedulable_threads(%rip)
	#APP
	movq	%cr3, %rcx
	#NO_APP
	movq	32(%rdi), %rax
	cmpq	%rax, %rcx
	je	.LBB0_11
# %bb.10:
	#APP
	movq	%rax, %cr3
	#NO_APP
.LBB0_11:
	xorl	%ecx, %ecx
	leaq	-24(%rsp), %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	je	.LBB0_13
# %bb.12:
	#APP
	.p2align	4, 0x90
.Ltmp5:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp6
	pause
	jmp	.Ltmp5
.Ltmp6:
	#NO_APP
	movl	$1, (%rax)
.LBB0_13:
	movq	%rdi, kernel_gs_base(%rip)
	leaq	2097152(%rdi), %rax
	movq	%rax, kernel_gs_base+36(%rip)
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	movq	16(%rdi), %rsp
	wrmsr
	lock		incq	old_schedulable_threads_num(%rip)
	jmpq	*24(%rdi)
	#NO_APP
.Ltmp0:                                 # Block address taken
# %bb.14:
	#APP
	nop
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
	subq	$24, %rsp
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	.p2align	4, 0x90
.LBB1_1:                                # =>This Inner Loop Header: Depth=1
	testq	%rax, %rax
	je	.LBB1_14
# %bb.2:                                #   in Loop: Header=BB1_1 Depth=1
	leaq	-1(%rax), %rcx
	lock		cmpxchgq	%rcx, schedulable_threads_num(%rip)
	jne	.LBB1_1
# %bb.3:
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	movq	%rsp, %rax
	movq	%rax, %rcx
	xchgq	%rcx, schedulable_threads_lock(%rip)
	testq	%rcx, %rcx
	je	.LBB1_5
# %bb.4:
	movq	%rax, 8(%rcx)
	#MEMBARRIER
	xorl	%eax, %eax
	#APP
	.p2align	4, 0x90
.Ltmp7:
	cmpl	(%rsp), %eax
	jne	.Ltmp8
	pause
	jmp	.Ltmp7
.Ltmp8:
	#NO_APP
.LBB1_5:
	#MEMBARRIER
	movq	schedulable_threads(%rip), %rsi
	movq	(%rsi), %rax
	cmpq	%rsi, %rax
	jne	.LBB1_7
# %bb.6:
	xorl	%eax, %eax
	jmp	.LBB1_8
.LBB1_7:
	movq	8(%rsi), %rcx
	movq	%rcx, 8(%rax)
	movq	8(%rsi), %rcx
	movq	%rax, (%rcx)
	movq	(%rsi), %rax
.LBB1_8:
	movq	%rax, schedulable_threads(%rip)
	xorl	%ecx, %ecx
	movq	%rsp, %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	jne	.LBB1_9
# %bb.10:
	#APP
	movq	%cr3, %rcx
	#NO_APP
	movq	32(%rsi), %rax
	cmpq	%rax, %rcx
	jne	.LBB1_11
.LBB1_12:
	movq	kernel_gs_base+8(%rip), %rax
	lock		decq	(%rax)
	jne	.LBB1_13
.LBB1_15:
	callq	abort@PLT
.LBB1_9:
	#APP
	.p2align	4, 0x90
.Ltmp9:
	movq	8(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp10
	pause
	jmp	.Ltmp9
.Ltmp10:
	#NO_APP
	movl	$1, (%rax)
	#APP
	movq	%cr3, %rcx
	#NO_APP
	movq	32(%rsi), %rax
	cmpq	%rax, %rcx
	je	.LBB1_12
.LBB1_11:
	#APP
	movq	%rax, %cr3
	#NO_APP
	movq	kernel_gs_base+8(%rip), %rax
	lock		decq	(%rax)
	je	.LBB1_15
.LBB1_13:
	movq	%rsi, kernel_gs_base(%rip)
	leaq	2097152(%rsi), %rax
	movq	%rax, kernel_gs_base+36(%rip)
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	movq	16(%rsi), %rsp
	wrmsr
	jmpq	*24(%rsi)
	#NO_APP
.LBB1_14:
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
.Lfunc_end1:
	.size	kkk2, .Lfunc_end1-kkk2
	.size	.Lkkk2$local, .Lfunc_end1-kkk2
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp1ubuntu2)"
	.section	".note.GNU-stack","",@progbits
