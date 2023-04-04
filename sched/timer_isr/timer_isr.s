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
    rdgsbaseq %rax
    swapgs
    pushq   %rdi
    pushq   %rax

    rdfsbaseq %rcx
    pushq   %rcx


    pushq   %rsi
    pushq   %r8
    pushq   %rbx
    pushq   %rbp
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
    # running_thread 的上下文已经保存完成






    # 切换至新线程%rsp
	movq	%rsp, 16(%rcx)
	leaq	.Ltmp0(%rip), %rax
	movq	%rax, 24(%rcx)
	movl	$0, -24(%rsp)
	movq	$0, -16(%rsp)
	leaq	-24(%rsp), %rax
	movq	%rax, %rdx

    # spin lock
	xchgq	%rdx, schedulable_threads_lock(%rip)
	testq	%rdx, %rdx
	je	.LBB0_2
	movq	%rax, 8(%rdx)
	xorl	%eax, %eax
	.p2align	4, 0x90
.Ltmp1:
	cmpl	-24(%rsp), %eax
	jne	.Ltmp2
	pause
	jmp	.Ltmp1
.Ltmp2:
.LBB0_2:

    # lock finish

	movq	schedulable_threads(%rip), %rdi
	testq	%rdi, %rdi
	je	.LBB0_3
	movq	(%rdi), %rax
	cmpq	%rdi, %rax
	je	.LBB0_7
	movq	%rax, (%rcx)
	movq	8(%rdi), %rdx
	movq	%rdx, 8(%rcx)
	movq	%rcx, 8(%rax)
	movq	8(%rcx), %rax
	movq	%rcx, (%rax)
	movq	(%rcx), %rcx
	jmp	.LBB0_9
.LBB0_3:
	# no new thread
	xorl	%ecx, %ecx
	leaq	-24(%rsp), %rax
    # spin unlock
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	je	.LBB0_5
	.p2align	4, 0x90
.Ltmp3:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp4
	pause
	jmp	.Ltmp3
.Ltmp4:
	movl	$1, (%rax)
.LBB0_5:
    # fast exit
    # 中断运行到此处，所有受损寄存器：%rax, %rcx, %rdx, %rdi
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	popq	%rsp
	addq	$104, %rsp
	wrmsr
	lock		incq	old_schedulable_threads_num(%rip)
	jmp	.Lpop4_iretq
.LBB0_7:
	movq	%rcx, (%rcx)
	movq	%rcx, 8(%rcx)
.LBB0_9:
	movq	%rcx, schedulable_threads(%rip)
	movq	%cr3, %rcx
	movq	32(%rdi), %rax
	cmpq	%rax, %rcx
	je	.LBB0_11
	movq	%rax, %cr3
.LBB0_11:
	xorl	%ecx, %ecx
	leaq	-24(%rsp), %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	je	.LBB0_13
	.p2align	4, 0x90
.Ltmp5:
	movq	-16(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp6
	pause
	jmp	.Ltmp5
.Ltmp6:
	movl	$1, (%rax)
.LBB0_13:
    # unlock finish
	movq	%rdi, %gs:0
	leaq	2097152(%rdi), %rax
	movq	%rax, %gs:36
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	movq	16(%rdi), %rsp
	wrmsr
	lock		incq	old_schedulable_threads_num(%rip)
	jmpq	*24(%rdi)
.Ltmp0:










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
    popq    %rbp
    popq    %rbx
    popq    %r8
    popq    %rsi

    popq    %rax
    wrfsbaseq %rax
    popq    %rcx
    swapgs
    wrgsbaseq %rcx
    testl   $0b11, 40(%rsp)
    jne     1f
    swapgs
1:

.Lpop4_iretq:
    popq    %rdi
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
	.p2align	4, 0x90
.LBB1_1:                                # =>This Inner Loop Header: Depth=1
	testq	%rax, %rax
	je	.LBB1_14
	leaq	-1(%rax), %rcx
	lock		cmpxchgq	%rcx, schedulable_threads_num(%rip)
	jne	.LBB1_1
    # 现在%rsp已经对齐16字节
	movl	$0, (%rsp)
	movq	$0, 8(%rsp)
	movq	%rsp, %rax
	movq	%rax, %rcx

    # spin lock
	xchgq	%rcx, schedulable_threads_lock(%rip)
	testq	%rcx, %rcx
	je	.LBB1_5
	movq	%rax, 8(%rcx)
	xorl	%eax, %eax
	.p2align	4, 0x90
.Ltmp7:
	cmpl	(%rsp), %eax
	jne	.Ltmp8
	pause
	jmp	.Ltmp7
.Ltmp8:
.LBB1_5:
    # lock finish

	movq	schedulable_threads(%rip), %rsi
	movq	(%rsi), %rax
	cmpq	%rsi, %rax
	jne	.LBB1_7
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
    # spin unlock
	xorl	%ecx, %ecx
	movq	%rsp, %rax
	lock		cmpxchgq	%rcx, schedulable_threads_lock(%rip)
	jne	.LBB1_9
    # unlock finish
	movq	%cr3, %rcx
	movq	32(%rsi), %rax
	cmpq	%rax, %rcx
	jne	.LBB1_11
.LBB1_12:
	movq	%gs:8, %rax
	lock		decq	(%rax)
	jne	.LBB1_13
.LBB1_15:
	callq	abort@PLT
	.p2align	4, 0x90
.LBB1_9:
.Ltmp9:
	movq	8(%rsp), %rax
	testq	%rax, %rax
	jne	.Ltmp10
	pause
	jmp	.Ltmp9
.Ltmp10:
	movl	$1, (%rax)
    # unlock finish
	movq	%cr3, %rcx
	movq	32(%rsi), %rax
	cmpq	%rax, %rcx
	je	.LBB1_12
.LBB1_11:
	movq	%rax, %cr3
	movq	%gs:8, %rax
	lock		decq	(%rax)
	je	.LBB1_15
.LBB1_13:
	movq	%rsi, %gs:0
	leaq	2097152(%rsi), %rax
	movq	%rax, %gs:36
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	movq	16(%rsi), %rsp
	wrmsr
	jmpq	*24(%rsi)
.LBB1_14:
	lock		incq	idle_cores_num(%rip)
	movl	$2059, %ecx                     # imm = 0x80B
	xorl	%eax, %eax
	xorl	%edx, %edx
	rdgsbaseq	%rsp
	addq	$65536, %rsp                    # imm = 0x10000
	wrmsr
	sti
	jmp	empty_loop

    .size   timer_isr, . - timer_isr
