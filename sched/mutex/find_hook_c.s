	.text
	.file	"find_hook_c.c"
	.section	.text.find_hook,"ax",@progbits
	.globl	find_hook                       # -- Begin function find_hook
	.p2align	4, 0x90
	.type	find_hook,@function
find_hook:                              # @find_hook
.Lfind_hook$local:
	.type	.Lfind_hook$local,@function
# %bb.0:
	subq	$24, %rsp
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	#APP
	cli
	#NO_APP
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	.p2align	4, 0x90
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	testq	%rax, %rax
	je	.LBB0_14
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	leaq	-1(%rax), %rcx
	lock		cmpxchgq	%rcx, schedulable_threads_num(%rip)
	jne	.LBB0_1
# %bb.3:
	movq	%rsp, %rax
	movq	%rax, %rcx
	xchgq	%rcx, schedulable_threads_lock(%rip)
	testq	%rcx, %rcx
	je	.LBB0_5
# %bb.4:
	movq	%rax, 8(%rcx)
	#MEMBARRIER
	xorl	%eax, %eax
	#APP
	.p2align	4, 0x90
.Ltmp0:
	cmpl	(%rsp), %eax
	jne	.Ltmp1
	pause
	jmp	.Ltmp0
.Ltmp1:
	#NO_APP
.LBB0_5:
	#MEMBARRIER
	movq	schedulable_threads(%rip), %rcx
	movq	(%rcx), %rax
	cmpq	%rcx, %rax
	jne	.LBB0_7
# %bb.6:
	xorl	%eax, %eax
	jmp	.LBB0_8
.LBB0_7:
	movq	8(%rcx), %rsi
	movq	%rsi, 8(%rax)
	movq	8(%rcx), %rsi
	movq	%rax, (%rsi)
	movq	(%rcx), %rax
.LBB0_8:
	movq	%rax, schedulable_threads(%rip)
	xorl	%esi, %esi
	movq	%rsp, %rax
	lock		cmpxchgq	%rsi, schedulable_threads_lock(%rip)
	jne	.LBB0_9
# %bb.10:
	#APP
	movq	%cr3, %rsi
	#NO_APP
	movq	32(%rcx), %rax
	cmpq	%rax, %rsi
	jne	.LBB0_11
.LBB0_12:
	lock		decq	(%rdx)
	jne	.LBB0_13
.LBB0_15:
	callq	abort@PLT
.LBB0_9:
	#APP
	.p2align	4, 0x90
.Ltmp2:
	movq	8(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp3
	pause
	jmp	.Ltmp2
.Ltmp3:
	#NO_APP
	movl	$1, (%rax)
	#APP
	movq	%cr3, %rsi
	#NO_APP
	movq	32(%rcx), %rax
	cmpq	%rax, %rsi
	je	.LBB0_12
.LBB0_11:
	#APP
	movq	%rax, %cr3
	#NO_APP
	lock		decq	(%rdx)
	je	.LBB0_15
.LBB0_13:
	movq	%rcx, kernel_gs_base(%rip)
	leaq	2097152(%rcx), %rax
	movq	%rax, kernel_gs_base+36(%rip)
	#APP
	movq	16(%rcx), %rsp
	jmpq	*24(%rcx)
	#NO_APP
.LBB0_14:
	lock		incq	idle_cores_num(%rip)
	#APP
	sti
	addq	$16, %rsp
	jmp	empty_loop
	#NO_APP
.Lfunc_end0:
	.size	find_hook, .Lfunc_end0-find_hook
	.size	.Lfind_hook$local, .Lfunc_end0-find_hook
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp1ubuntu2)"
	.section	".note.GNU-stack","",@progbits
