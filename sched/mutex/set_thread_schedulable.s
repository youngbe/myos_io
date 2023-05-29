	.section	.text.set_thread_schedulable,"ax",@progbits
	.globl	set_thread_schedulable          # -- Begin function set_thread_schedulable
	.p2align	4, 0x90
	.type	set_thread_schedulable,@function
set_thread_schedulable:                 # @set_thread_schedulable
	movl	$802851, %eax                   # imm = 0xC4023
	xorl	%edx, %edx
	movl	$2096, %ecx                     # imm = 0x830
    movq    %rdx, (%rdi)
	movq	%rdi, %rsi
	leaq	schedulable_threads(%rip), %r8
	xchgq	%rsi, schedulable_threads+8(%rip)
	testq	%rsi, %rsi
	cmovneq	%rsi, %r8
	movq	%rdi, (%r8)
	lock		incq	schedulable_threads_num(%rip)
    SERIALIZE
    cmpq    %rdx, schedulable_threads_num(%rip)
    je  .LBB0_2
	cmpq	%rdx, idle_cores_num(%rip)
	je	.LBB0_2
	wrmsr
.LBB0_2:
	lock		incq	old_schedulable_threads_num(%rip)
	retq
.Lfunc_end0:
	.size	set_thread_schedulable, .Lfunc_end0-set_thread_schedulable



	.section	.text.cli_set_thread_schedulable,"ax",@progbits
	.globl	cli_set_thread_schedulable      # -- Begin function cli_set_thread_schedulable
	.p2align	4, 0x90
	.type	cli_set_thread_schedulable,@function
cli_set_thread_schedulable:             # @cli_set_thread_schedulable
	movl	$802851, %eax                   # imm = 0xC4023
	xorl	%edx, %edx
	movl	$2096, %ecx                     # imm = 0x830
    movq    %rdx, (%rdi)
	movq	%rdi, %rsi
	leaq	schedulable_threads(%rip), %r8
    cli
	xchgq	%rsi, schedulable_threads+8(%rip)
	testq	%rsi, %rsi
	cmovneq	%rsi, %r8
	movq	%rdi, (%r8)
	lock		incq	schedulable_threads_num(%rip)
    SERIALIZE
    cmpq    %rdx, schedulable_threads_num(%rip)
    je  .LBB0_3
	cmpq	%rdx, idle_cores_num(%rip)
	je	.LBB0_3
	wrmsr
.LBB0_3:
	lock		incq	old_schedulable_threads_num(%rip)
    sti
	retq
.Lfunc_end1:
	.size	cli_set_thread_schedulable, .Lfunc_end1-cli_set_thread_schedulable
