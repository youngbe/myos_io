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
    # %rdx == current_thread
    # %rdi == mtx
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushfq
	movq	(%rsp), %rcx
	addq	%rsi, %r9
	movq	8(%r9), %r8
	leaq	.Ltmp0(%rip), %rax
	movl	$1, %esi
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_2
	cli
.LBB0_2:
	movq	%rsp, 16(%rdx)
	movq	%rax, 24(%rdx)
	.p2align	4, 0x90
.LBB0_3:                                # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, 24(%rdi)
	jne	.LBB0_3
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, (%rdi)
	jne	.LBB0_5
	movq	$0, 24(%rdi)
	testl	$512, %ecx                      # imm = 0x200
	je	.LBB0_22
	sti
.LBB0_22:
	addq	$8, %rsp
.LBB0_24:
	xorl	%eax, %eax
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	vzeroupper
	retq
.LBB0_5:
	movq	16(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_7
	movq	64(%rax), %rcx
	movq	%rcx, 64(%rdx)
	movq	%rax, 56(%rdx)
	movq	%rdx, 64(%rax)
	movq	64(%rdx), %rax
	addq	$56, %rax
	jmp	.LBB0_8
.LBB0_7:
	leaq	16(%rdi), %rax
	movq	%rdx, 64(%rdx)
	movq	%rdx, 56(%rdx)
.LBB0_8:
	movq	%rdx, (%rax)
	movl	$1, %ecx
	.p2align	4, 0x90
.LBB0_9:                                # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	jne	.LBB0_9
	movq	schedulable_threads(%rip), %rax
	testq	%rax, %rax
	je	.LBB0_11
	movq	(%rax), %rcx
	cmpq	%rax, %rcx
	je	.LBB0_13
	movq	8(%rax), %rdx
	movq	%rdx, 8(%rcx)
	movq	8(%rax), %rdx
	movq	%rcx, (%rdx)
	movq	(%rax), %rcx
	jmp	.LBB0_15
.LBB0_11:
	movq	$0, schedulable_threads_lock(%rip)
	leaq	kernel_pt0(%rip), %rax
	movq	%rax, %cr3
	movq	$0, 24(%rdi)
	movq	%r8, %rsp
	sti
	jmp	empty_loop
.LBB0_13:
	xorl	%ecx, %ecx
.LBB0_15:
	movq	%rcx, schedulable_threads(%rip)
	movq	$0, schedulable_threads_lock(%rip)
	movq	%cr3, %rdx
	movq	32(%rax), %rcx
	cmpq	%rcx, %rdx
	je	.LBB0_17
	movq	%rcx, %cr3
.LBB0_17:
	movq	$0, 24(%rdi)
	movq	%rax, (%r9)
	movq	16(%rax), %rsp
	jmpq	*24(%rax)
.Ltmp0:                                 # Block address taken
	popfq
	jmp	.LBB0_24
.Lfunc_end0:
	.size	mtx_lock, .Lfunc_end0-mtx_lock
                                        # -- End function
	.ident	"Ubuntu clang version 15.0.6"
	.section	".note.GNU-stack","",@progbits
