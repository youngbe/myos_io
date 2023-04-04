	.text
	.file	"mtx_lock3.c"
	.globl	mtx_lock                        # -- Begin function mtx_lock
	.p2align	4, 0x90
	.type	mtx_lock,@function
mtx_lock:                               # @mtx_lock
	movq	core_res(%rip), %r9
	movl	$2050, %ecx                     # imm = 0x802
	rdmsr
	movl	%eax, %esi
	shlq	$4, %rsi
	movq	(%r9,%rsi), %rdx
    # %r9 == core_res, %rdx == running_thread
    # %rdi == mtx
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, (%rdi)
	jne	1f
    # trylock success: owner == NULL
    retq
1:
	cmpq	%rax, %rdx
	jne	1f
    # current_thread(%rdx) == owner(%rax)
	movq	8(%rdi), %rax
	movl	$2, %ecx
	incq	%rax
	cmpq	%rcx, %rax
	jae	2f
    # error: %rax == (count + 1) < 2   -> count == 0 || count == SIZE_MAX
    movl    %ecx, %eax
    retq
2:
    # set count: %rax == count + 1
	movq	%rax, 8(%rdi)
    xorl    %eax, %eax
    # trylock success: count++
	retq
1:


    # %rsi == core_id*16
    # %r9 == core_res
	addq	%rsi, %r9
	movq	8(%r9), %r8
    # %rdx == current_thread
    # %r8 == empty_stack_rsp
    # %r9 == _core_res
    # %rdi == mtx

	pushfq
	pushq	%rbx
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	pushq	%rbp
	cli
	movq	%rsp, 16(%rdx)
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%rdx)
	movl	$1, %esi
	.p2align	4, 0x90
.LBB0_5:                                # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 24(%rdi)
	jne	.LBB0_5
# %bb.6:
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, (%rdi)
	jne	.LBB0_10
# %bb.7:
	movq	$0, 24(%rdi)
	testb	$1, %cl
	je	.LBB0_9
# %bb.8:
	#APP
	sti
	#NO_APP
.LBB0_9:
	#APP
	addq	$10, %rsp
	#NO_APP
	xorl	%ecx, %ecx
	movl	%ecx, %eax
	retq
.LBB0_10:
	movq	16(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_12
# %bb.11:
	movq	64(%rax), %rcx
	movq	%rcx, 64(%rdx)
	movq	%rax, 56(%rdx)
	movq	%rdx, 64(%rax)
	movq	64(%rdx), %rax
	addq	$56, %rax
	jmp	.LBB0_13
.LBB0_12:
	leaq	16(%rdi), %rax
	movq	%rdx, 64(%rdx)
	movq	%rdx, 56(%rdx)
.LBB0_13:
	movq	%rdx, (%rax)
	movl	$1, %ecx
	.p2align	4, 0x90
.LBB0_14:                               # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	jne	.LBB0_14
# %bb.15:
	movq	schedulable_threads(%rip), %rax
	testq	%rax, %rax
	je	.LBB0_16
# %bb.17:
	movq	(%rax), %rcx
	cmpq	%rax, %rcx
	je	.LBB0_18
# %bb.19:
	movq	8(%rax), %rdx
	movq	%rdx, 8(%rcx)
	movq	8(%rax), %rdx
	movq	%rcx, (%rdx)
	movq	(%rax), %rcx
	jmp	.LBB0_20
.LBB0_16:
	movq	$0, schedulable_threads_lock(%rip)
	leaq	kernel_pt0(%rip), %rax
	#APP
	movq	%rax, %cr3
	#NO_APP
	movq	$0, 24(%rdi)
	#APP
	movq	%r8, %rsp
	sti
	jmp	empty_loop
	#NO_APP
	xorl	%ecx, %ecx
	movl	%ecx, %eax
	retq
.LBB0_18:
	xorl	%ecx, %ecx
.LBB0_20:
	movq	%rcx, schedulable_threads(%rip)
	movq	$0, schedulable_threads_lock(%rip)
	#APP
	movq	%cr3, %rdx
	#NO_APP
	movq	32(%rax), %rcx
	cmpq	%rcx, %rdx
	je	.LBB0_22
# %bb.21:
	#APP
	movq	%rcx, %cr3
	#NO_APP
.LBB0_22:
	movq	$0, 24(%rdi)
	movq	%rax, (%r9)
	#APP
	movq	16(%rax), %rsp
	jmpq	*24(%rax)
	#NO_APP
	xorl	%ecx, %ecx
	movl	%ecx, %eax
	retq
.Lfunc_end0:
	.size	mtx_lock, .Lfunc_end0-mtx_lock
                                        # -- End function
	.ident	"Ubuntu clang version 15.0.6"
	.section	".note.GNU-stack","",@progbits
