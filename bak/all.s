	.text
	.file	"mtx_lock_release_all.c"
	.globl	mtx_lockx                       # -- Begin function mtx_lockx
	.p2align	4, 0x90
	.type	mtx_lockx,@function
mtx_lockx:                              # @mtx_lockx
.Lmtx_lockx$local:
# %bb.0:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	movq	%rdx, %r8
	#APP
	pushfq
	movq	(%rsp), %r9
	#NO_APP
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_2
# %bb.1:
	#APP
	cli
	#NO_APP
.LBB0_2:
	#APP
	movq	%rsp, 16(%r8)
	#NO_APP
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%r8)
	movl	$2050, %ecx                     # imm = 0x802
	#APP
	rdmsr
	#NO_APP
	movl	%eax, %edx
	shlq	$4, %rdx
	movq	8(%rsi,%rdx), %r10
	movl	$1, %ecx
	.p2align	4, 0x90
.LBB0_3:                                # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rcx, 24(%rdi)
	jne	.LBB0_3
# %bb.4:
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, (%rdi)
	jne	.LBB0_5
# %bb.20:
	movq	$0, 24(%rdi)
	testl	$512, %r9d                      # imm = 0x200
	je	.LBB0_22
# %bb.21:
	#APP
	sti
	#NO_APP
.LBB0_22:
	#APP
	addq	$8, %rsp
	#NO_APP
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
# %bb.6:
	movq	64(%rax), %rcx
	movq	%rcx, 64(%r8)
	movq	%rax, 56(%r8)
	movq	%r8, 64(%rax)
	movq	64(%r8), %rax
	addq	$56, %rax
	jmp	.LBB0_8
.LBB0_7:
	leaq	16(%rdi), %rax
	movq	%r8, 64(%r8)
	movq	%r8, 56(%r8)
.LBB0_8:
	addq	%rdx, %rsi
	movq	%r8, (%rax)
	movl	$1, %ecx
	.p2align	4, 0x90
.LBB0_9:                                # =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	jne	.LBB0_9
# %bb.10:
	movq	schedulable_threads(%rip), %rax
	testq	%rax, %rax
	je	.LBB0_11
# %bb.12:
	movq	(%rax), %rcx
	cmpq	%rax, %rcx
	je	.LBB0_13
# %bb.14:
	movq	8(%rax), %rdx
	movq	%rdx, 8(%rcx)
	movq	8(%rax), %rdx
	movq	%rcx, (%rdx)
	movq	(%rax), %rcx
	jmp	.LBB0_15
.LBB0_11:
	movq	$0, schedulable_threads_lock(%rip)
	leaq	kernel_pt0(%rip), %rax
	#APP
	movq	%rax, %cr3
	#NO_APP
	movq	$0, 24(%rdi)
	movq	$0, (%rsi)
	#APP
	movq	%r10, %rsp
	sti
	jmp	empty_loop
	#NO_APP
# %bb.19:
.LBB0_13:
	xorl	%ecx, %ecx
.LBB0_15:
	movq	%rcx, schedulable_threads(%rip)
	movq	$0, schedulable_threads_lock(%rip)
	#APP
	movq	%cr3, %rdx
	#NO_APP
	movq	32(%rax), %rcx
	cmpq	%rcx, %rdx
	je	.LBB0_17
# %bb.16:
	#APP
	movq	%rcx, %cr3
	#NO_APP
.LBB0_17:
	movq	$0, 24(%rdi)
	movq	%rax, (%rsi)
	#APP
	movq	16(%rax), %rsp
	jmpq	*24(%rax)
	#NO_APP
# %bb.18:
.Ltmp0:                                 # Block address taken
.LBB0_23:
	#APP
	popfq
	#NO_APP
	jmp	.LBB0_24
.Lfunc_end0:
	.size	mtx_lockx, .Lfunc_end0-mtx_lockx
                                        # -- End function
	.globl	mtx_lock                        # -- Begin function mtx_lock
	.p2align	4, 0x90
	.type	mtx_lock,@function
mtx_lock:                               # @mtx_lock
.Lmtx_lock$local:
# %bb.0:
	pushq	%rbx
	movq	core_res(%rip), %rsi
	movl	$2050, %ecx                     # imm = 0x802
	#APP
	pushfq
	popq	%rbx
	andq	$512, %rbx                      # imm = 0x200
	je	.Ltmp1
	cli
.Ltmp1:
	rdmsr
	shlq	$4, %rax
	movq	(%rsi,%rax), %rax
	testq	%rbx, %rbx
	je	.Ltmp2
	sti
.Ltmp2:
	#NO_APP
	movq	%rax, %rdx
	xorl	%ebx, %ebx
	xorl	%eax, %eax
	lock		cmpxchgq	%rdx, (%rdi)
	je	.LBB1_5
# %bb.1:
	cmpq	%rax, %rdx
	je	.LBB1_2
# %bb.4:
	callq	.Lmtx_lockx$local
.LBB1_5:
	movl	%ebx, %eax
	popq	%rbx
	retq
.LBB1_2:
	movq	8(%rdi), %rax
	movl	$2, %ebx
	incq	%rax
	cmpq	$2, %rax
	jb	.LBB1_5
# %bb.3:
	movq	%rax, 8(%rdi)
	xorl	%ebx, %ebx
	movl	%ebx, %eax
	popq	%rbx
	retq
.Lfunc_end1:
	.size	mtx_lock, .Lfunc_end1-mtx_lock
                                        # -- End function
	.ident	"Ubuntu clang version 15.0.6"
	.section	".note.GNU-stack","",@progbits
