    .section .text.timer_isr, "ax", @progbits
    .globl timer_isr
    .globl xsave_area_size
    .p2align    4, 0x90
    .type   timer_isr,@function
timer_isr:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx

    # 获取running_thread，判断是不是跑空线程，或者是不是被kill线程
    # 如果是被kill线程或者空线程，则不需要保存上下文
    testl       $0b11, 32(%rsp)
    je          1f
    # 从用户态进来，running_thread不可能是NULL
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
    jz          empty_thread_switch_isr
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
    pushq   %rax

    rdfsbase %rcx
    pushq   %rcx


    pushq   %rbx
    pushq   %rbp
    pushq   %r8
    pushq   %r9
    pushq   %r10
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
	movq  %rsp, 16(%rcx)
	leaq  .Lreturn(%rip), %rax
	#NO_APP
	movq	%rax, 24(%rcx)
	movq	40(%rcx), %rsi
	movq	%rsi, %rdx
	andq	$-2, %rdx
	testb	$1, %sil
	jne	.LBB0_2
# %bb.1:
	lock		incq	(%rdx)
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
	movq	32(%rdi), %rax
	testq	%rax, %rax
	je	.LBB0_26
# %bb.21:
	cmpq	%rdx, 40(%rdi)
	je	.LBB0_23
# %bb.22:
	#APP
	movq  %rax, %cr3
	#NO_APP
.LBB0_23:
	lock		decq	(%rdx)
	jne	.LBB0_25
# %bb.24:
	#APP
	jmp abort
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
.LBB0_25:
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	jmp	.LBB0_27
.LBB0_17:
	testb	$1, %sil
	jne	.LBB0_19
# %bb.18:
	lock		decq	(%rdx)
.LBB0_19:
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	popq   %rsp
	addq   $96, %rsp
	wrmsr
	lock   incq    old_schedulable_threads_num(%rip)
	jmp    .Lpop5_iretq
	#NO_APP
.LBB0_26:
	orq	$1, %rsi
	movq	%rsi, 40(%rdi)
.LBB0_27:
	movq	%rdi, %gs:0
	#APP
	movq   16(%rdi), %rsp
	#NO_APP
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	wrmsr
	#NO_APP
	lock		incq	old_schedulable_threads_num(%rip)
	#APP
	jmpq   *24(%rdi)

	











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
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rbp
    popq    %rbx

    popq    %rax
    wrfsbase %rax
    popq    %rcx
    swapgs
    wrgsbase %rcx
    testl   $0b11, 48(%rsp)
    jne     1f
    swapgs
1:

.Lpop5_iretq:
    popq    %rdi
    popq    %rsi
.Lpop3_iretq:
    popq    %rdx
    popq    %rcx
    popq    %rax
    iretq

.Lfast_exit_new:
    # 获取到 old_schedulable_threads_num <= 0
    xorl    %eax, %eax
    xorl    %edx, %edx
    movl    $0x80b, %ecx
    wrmsr
    lock incq   old_schedulable_threads_num(%rip)
    jmp .Lpop3_iretq

# 保证进入时已经关闭中断
# %rsp对齐16字节且已申请栈有16字节空间
# 返回时会写入 EOI
    .globl empty_thread_switch_isr
    .type empty_thread_switch_isr, @function
    .p2align 4, 0x90
empty_thread_switch_isr:
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
	rdgsbase %rsp
	addq   $65536, %rsp
	wrmsr
	sti
	jmp   empty_loop
	#NO_APP
.LBB1_5:
	#APP
	movq     %rsp, %rcx
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
	movq	schedulable_threads(%rip), %rsi
	movq	8(%rsi), %rdx
	cmpq	%rsi, %rdx
	je	.LBB1_11
# %bb.12:
	movq	(%rsi), %rax
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
	movq	%gs:8, %rax
	movq	32(%rsi), %rcx
	testq	%rcx, %rcx
	je	.LBB1_23
# %bb.18:
	cmpq	%rax, 40(%rsi)
	je	.LBB1_20
# %bb.19:
	#APP
	movq  %rcx, %cr3
	#NO_APP
.LBB1_20:
	lock		decq	(%rax)
	jne	.LBB1_22
# %bb.21:
	#APP
	jmp abort
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
	#APP
	nop
	#NO_APP
.LBB1_22:
	leaq	2097152(%rsi), %rax
	movq	%rax, %gs:36
	jmp	.LBB1_24
.LBB1_23:
	orq	$1, %rax
	movq	%rax, 40(%rsi)
.LBB1_24:
	movq	%rsi, %gs:0
	#APP
	movq   16(%rsi), %rsp
	#NO_APP
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	#APP
	wrmsr
	#NO_APP
	#APP
	jmpq   *24(%rsi)


    .size   timer_isr, . - timer_isr
