	.text
	.file	"empty_switch_c.c"
	.section	.text.switch_to,"ax",@progbits
	.globl	switch_to                       # -- Begin function switch_to
	.p2align	4, 0x90
	.type	switch_to,@function
switch_to:                              # @switch_to
.Lswitch_to$local:
	.type	.Lswitch_to$local,@function
# %bb.0:
	movq	32(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_6
# %bb.1:
	cmpq	%rsi, 40(%rdi)
	je	.LBB0_3
# %bb.2:
	#APP
	movq	%rax, %cr3
	#NO_APP
.LBB0_3:
	lock		decq	(%rsi)
	jne	.LBB0_5
# %bb.4:
	#APP
	leaq	-16(%rsp), %rax
	#NO_APP
	#APP
	jmp	abort
	#NO_APP
.LBB0_5:
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	movq	%rdi, %gs:0
	#APP
	movq	16(%rdi), %rsp
	jmpq	*24(%rdi)
	#NO_APP
.LBB0_6:
	orq	$1, %rsi
	movq	%rsi, 40(%rdi)
	movq	%rdi, %gs:0
	#APP
	movq	16(%rdi), %rsp
	jmpq	*24(%rdi)
	#NO_APP
.Lfunc_end0:
	.size	switch_to, .Lfunc_end0-switch_to
	.size	.Lswitch_to$local, .Lfunc_end0-switch_to
                                        # -- End function
	.section	.text.switch_to_interrupt,"ax",@progbits
	.globl	switch_to_interrupt             # -- Begin function switch_to_interrupt
	.p2align	4, 0x90
	.type	switch_to_interrupt,@function
switch_to_interrupt:                    # @switch_to_interrupt
.Lswitch_to_interrupt$local:
	.type	.Lswitch_to_interrupt$local,@function
# %bb.0:
	movq	32(%rdi), %rax
	testq	%rax, %rax
	je	.LBB1_6
# %bb.1:
	cmpq	%rsi, 40(%rdi)
	je	.LBB1_3
# %bb.2:
	#APP
	movq	%rax, %cr3
	#NO_APP
.LBB1_3:
	lock		decq	(%rsi)
	jne	.LBB1_5
# %bb.4:
	#APP
	leaq	-16(%rsp), %rax
	#NO_APP
	#APP
	jmp	abort
	#NO_APP
.LBB1_5:
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	jmp	.LBB1_7
.LBB1_6:
	orq	$1, %rsi
	movq	%rsi, 40(%rdi)
.LBB1_7:
	movq	%rdi, %gs:0
	#APP
	movq	16(%rdi), %rsp
	#NO_APP
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	wrmsr
	#NO_APP
	#APP
	jmpq	*24(%rdi)
	#NO_APP
.Lfunc_end1:
	.size	switch_to_interrupt, .Lfunc_end1-switch_to_interrupt
	.size	.Lswitch_to_interrupt$local, .Lfunc_end1-switch_to_interrupt
                                        # -- End function
	.section	.text.switch_to_empty,"ax",@progbits
	.globl	switch_to_empty                 # -- Begin function switch_to_empty
	.p2align	4, 0x90
	.type	switch_to_empty,@function
switch_to_empty:                        # @switch_to_empty
.Lswitch_to_empty$local:
	.type	.Lswitch_to_empty$local,@function
# %bb.0:
	lock		incq	idle_cores_num(%rip)
	movq	$0, %gs:0
	movq	%rdi, %gs:8
	#MEMBARRIER
	#APP
	sti
	#NO_APP
	#MEMBARRIER
	#APP
	movq	%rsp, %rcx
	#NO_APP
	movl	$0, (%rcx)
	movq	$0, 8(%rcx)
	#MEMBARRIER
	#APP
	cli
	#NO_APP
	#MEMBARRIER
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	testq	%rax, %rax
	je	.LBB2_4
# %bb.1:
	movq	%rdi, %rsi
	.p2align	4, 0x90
.LBB2_2:                                # =>This Inner Loop Header: Depth=1
	leaq	-1(%rax), %rdx
	lock		cmpxchgq	%rdx, schedulable_threads_num(%rip)
	je	.LBB2_5
# %bb.3:                                #   in Loop: Header=BB2_2 Depth=1
	#APP
	pause
	#NO_APP
	testq	%rax, %rax
	jne	.LBB2_2
.LBB2_4:
	lock		incq	idle_cores_num(%rip)
	#APP
	sti
	addq	$16, %rsp
	jmp	empty_loop
	#NO_APP
.LBB2_5:
	#MEMBARRIER
	movq	%rcx, %rax
	xchgq	%rax, schedulable_threads_lock(%rip)
	testq	%rax, %rax
	je	.LBB2_6
# %bb.7:
	movq	%rcx, 8(%rax)
	#MEMBARRIER
	movl	(%rcx), %eax
	testl	%eax, %eax
	jne	.LBB2_10
	.p2align	4, 0x90
.LBB2_8:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movl	(%rcx), %eax
	testl	%eax, %eax
	je	.LBB2_8
	jmp	.LBB2_10
.LBB2_6:
	#MEMBARRIER
.LBB2_10:
	movq	schedulable_threads(%rip), %rdi
	movq	8(%rdi), %rdx
	cmpq	%rdi, %rdx
	je	.LBB2_11
# %bb.12:
	movq	(%rdi), %rax
	movq	%rdx, 8(%rax)
	jmp	.LBB2_13
.LBB2_11:
	xorl	%eax, %eax
.LBB2_13:
	movq	%rax, schedulable_threads(%rip)
	xorl	%edx, %edx
	movq	%rcx, %rax
	lock		cmpxchgq	%rdx, schedulable_threads_lock(%rip)
	je	.LBB2_17
	.p2align	4, 0x90
# %bb.15:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.LBB2_16
.LBB2_14:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.LBB2_14
.LBB2_16:
	movl	$1, (%rax)
.LBB2_17:
	#MEMBARRIER
	#APP
	jmp	switch_to
	#NO_APP
.Lfunc_end2:
	.size	switch_to_empty, .Lfunc_end2-switch_to_empty
	.size	.Lswitch_to_empty$local, .Lfunc_end2-switch_to_empty
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
