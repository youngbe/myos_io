	.section	.text.empty_switch_to_empty_interrupt,"ax",@progbits
	.globl	empty_switch_to_empty_interrupt # -- Begin function empty_switch_to_empty_interrupt
	.p2align	4, 0x90
	.type	empty_switch_to_empty_interrupt,@function
empty_switch_to_empty_interrupt:        # @empty_switch_to_empty_interrupt
    rdgsbase    %rsp
    addq        $65536 - 16, %rsp
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
    wrmsr
    jmp         .Lswitch_to_empty_part2
.Lfunc_end0:
	.size	empty_switch_to_empty_interrupt, .Lfunc_end0-empty_switch_to_empty_interrupt

	.section	.text.switch_to,"ax",@progbits
	.globl	switch_to                       # -- Begin function switch_to
	.p2align	4, 0x90
	.type	switch_to,@function
switch_to:                              # @switch_to
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
	jmp	abort
	#NO_APP
.LBB1_5:
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	movq	%rdi, %gs:0
	#APP
	movq	16(%rdi), %rsp
	jmpq	*24(%rdi)
	#NO_APP
.LBB1_6:
	orq	$1, %rsi
	movq	%rsi, 40(%rdi)
	movq	%rdi, %gs:0
	#APP
	movq	16(%rdi), %rsp
	jmpq	*24(%rdi)
	#NO_APP
.Lfunc_end1:
	.size	switch_to, .Lfunc_end1-switch_to
                                        # -- End function
	.section	.text.switch_to_interrupt,"ax",@progbits
	.globl	switch_to_interrupt             # -- Begin function switch_to_interrupt
	.p2align	4, 0x90
	.type	switch_to_interrupt,@function
switch_to_interrupt:                    # @switch_to_interrupt
	movq	32(%rdi), %rax
	testq	%rax, %rax
	je	.LBB2_6
# %bb.1:
	cmpq	%rsi, 40(%rdi)
	je	.LBB2_3
# %bb.2:
	#APP
	movq	%rax, %cr3
	#NO_APP
.LBB2_3:
	lock		decq	(%rsi)
	jne	.LBB2_5
# %bb.4:
	#APP
	jmp	abort
	#NO_APP
.LBB2_5:
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	jmp	.LBB2_7
.LBB2_6:
	orq	$1, %rsi
	movq	%rsi, 40(%rdi)
.LBB2_7:
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
.Lfunc_end2:
	.size	switch_to_interrupt, .Lfunc_end2-switch_to_interrupt
                                        # -- End function
	.section	.text.switch_to_empty,"ax",@progbits
	.globl	switch_to_empty                 # -- Begin function switch_to_empty
	.p2align	4, 0x90
	.type	switch_to_empty,@function
switch_to_empty:                        # @switch_to_empty
	lock		incq	idle_cores_num(%rip)
	movq	$0, %gs:0
	movq	%rdi, %gs:8
.Lswitch_to_empty_part2:
	#APP
	sti
	#NO_APP
	#APP
	movq	%rsp, %rcx
	#NO_APP
	movl	$0, (%rcx)
	movq	$0, 8(%rcx)
	#APP
	cli
	#NO_APP
	lock		decq	idle_cores_num(%rip)
	movq	schedulable_threads_num(%rip), %rax
	testq	%rax, %rax
	je	.LBB3_4
# %bb.1:
	movq	%rdi, %rsi
	.p2align	4, 0x90
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
	leaq	-1(%rax), %rdx
	lock		cmpxchgq	%rdx, schedulable_threads_num(%rip)
	je	.LBB3_5
# %bb.3:                                #   in Loop: Header=BB3_2 Depth=1
	#APP
	pause
	#NO_APP
	testq	%rax, %rax
	jne	.LBB3_2
.LBB3_4:
	lock		incq	idle_cores_num(%rip)
	#APP
	sti
	addq	$16, %rsp
	jmp	empty_loop
	#NO_APP
.LBB3_5:
	movq	%rcx, %rax
	xchgq	%rax, schedulable_threads_lock(%rip)
	testq	%rax, %rax
	je	.LBB3_9
# %bb.6:
	movq	%rcx, 8(%rax)
	cmpl	$0, (%rcx)
	jne	.LBB3_9
	.p2align	4, 0x90
.LBB3_7:                                # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	cmpl	$0, (%rcx)
	je	.LBB3_7
.LBB3_9:
	movq	schedulable_threads+8(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB3_10
	.p2align	4, 0x90
# %bb.12:
	movq	schedulable_threads(%rip), %rdx
	testq	%rdx, %rdx
	jne	.LBB3_13
.LBB3_11:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	schedulable_threads(%rip), %rdx
	testq	%rdx, %rdx
	je	.LBB3_11
.LBB3_13:
	cmpq	%rdi, %rdx
	jne	.LBB3_16
# %bb.14:
	movq	$0, schedulable_threads(%rip)
	xorl	%r8d, %r8d
	movq	%rdi, %rax
	lock		cmpxchgq	%r8, schedulable_threads+8(%rip)
	je	.LBB3_18
	.p2align	4, 0x90
.LBB3_16:
	movq	(%rdx), %rax
	testq	%rax, %rax
	jne	.LBB3_17
.LBB3_15:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	(%rdx), %rax
	testq	%rax, %rax
	je	.LBB3_15
.LBB3_17:
	movq	%rax, schedulable_threads(%rip)
	movq	%rdx, %rdi
.LBB3_18:
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%rcx, %rax
	jne	.LBB3_21
	jmp	.LBB3_19
.LBB3_10:
	xorl	%edi, %edi
	movq	schedulable_threads_lock(%rip), %rax
	cmpq	%rcx, %rax
	jne	.LBB3_21
.LBB3_19:
	xorl	%edx, %edx
	movq	%rcx, %rax
	lock		cmpxchgq	%rdx, schedulable_threads_lock(%rip)
	je	.LBB3_23
	.p2align	4, 0x90
.LBB3_21:
	movq	8(%rcx), %rax
	testq	%rax, %rax
	jne	.LBB3_22
.LBB3_20:                               # =>This Inner Loop Header: Depth=1
	#APP
	pause
	#NO_APP
	movq	8(%rcx), %rax
	testq	%rax, %rax
	je	.LBB3_20
.LBB3_22:
	movl	$1, (%rax)
.LBB3_23:
	#APP
	jmp	switch_to
	#NO_APP
.Lfunc_end3:
	.size	switch_to_empty, .Lfunc_end3-switch_to_empty
