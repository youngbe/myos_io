    .section .text.timer_isr, "ax", @progbits
    .globl timer_isr
    .globl xsave_area_size
    .p2align    4, 0x90
    .type   timer_isr,@function
timer_isr:
    pushq       %rdx

    # 获取running_thread，判断是不是跑空线程，或者是不是被kill线程
    # 如果是被kill线程或者空线程，则不需要保存上下文
    testl       $0b11, 16(%rsp)
    je          1f
    # 从用户态进来，running_thread不可能是NULL
    pushq       %rax
    pushq       %rcx
    movq        $-1, %rax
    lock xaddq  %rax, old_schedulable_threads_num(%rip)
    testq       %rax, %rax
    jle         .Lfast_exit_new
    movq        %gs:0, %rdx
    jmp         2f
1:
    # 在内核态，running_thread可能是NULL
    movq        %gs:0, %rdx
    testq       %rdx, %rdx
    jz          empty_switch_to_empty_interrupt
    pushq       %rax
    pushq       %rcx
    movq        $-1, %rax
    lock xaddq  %rax, old_schedulable_threads_num(%rip)
    testq       %rax, %rax
    jle         .Lfast_exit_new
    swapgs
2:





    # 完整地保存上下文
    rdgsbase %rax
    swapgs
    pushq   %rsi
    pushq   %rdi
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %rax

    rdfsbase %rcx
    pushq   %rcx


    pushq   %rbx
    pushq   %rbp
    pushq   %r11
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    # 运行至此处，破坏寄存器：%rax, %rcx, %rdx
    # %rdx == running_thread


    movq    %rsp, %rax

    #movq   xsave_area_size, %rcx
    .byte   0x48, 0xc7, 0xc1
xsave_area_size:
    .long   0

    subq    %rcx, %rsp
    andq    $-64, %rsp
    movq    %rsp, %rdi
    pushq   %rax
    cld
    xorq    %rax, %rax
    rep stosb

    movq    %rdx, %rcx

    # 运行xsave，破坏寄存器：%rdx, %rax
    movl    $-1, %eax
    movl    %eax, %edx
    xsave   8(%rsp)


    # 运行至此处：破坏寄存器：%rax, %rcx, %rdx, %rdi
    # %rcx == running_thread
    # running_thread 的上下文已经基本保存完成






    # 切换至新线程%rsp
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
	movq	$0, schedulable_threads(%rip)
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
	movq	%rax, schedulable_threads(%rip)
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
	addq	$72, %rsp
	wrmsr
	#NO_APP
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmp	.Lpop8_iretq

	











	.p2align 4, 0x90
.Lreturn:
    popq    %rbp

    movl    $-1, %eax
    movl    %eax, %edx
    xrstor  (%rsp)

    movq    %rbp, %rsp
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %r11
    popq    %rbp
    popq    %rbx

    popq    %rax
    wrfsbase %rax
    popq    %rcx
    swapgs
    wrgsbase %rcx
    testl   $0b11, 72(%rsp)
    jne     1f
    swapgs
1:

.Lpop8_iretq:
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rdi
    popq    %rsi
.Lpop3_iretq:
    popq    %rcx
    popq    %rax
    popq    %rdx
    iretq

.Lfast_exit_new:
    # 获取到 old_schedulable_threads_num <= 0
    xorl    %eax, %eax
    xorl    %edx, %edx
    movl    $0x80b, %ecx
    wrmsr
    lock incq   old_schedulable_threads_num(%rip)
    jmp .Lpop3_iretq

    .size   timer_isr, . - timer_isr
