	.section	.text.keyboard_thread,"ax",@progbits
	.globl	keyboard_thread                 # -- Begin function keyboard_thread
	.p2align	4, 0x90
	.type	keyboard_thread,@function
keyboard_thread:                        # @keyboard_thread
.Lkeyboard_thread$local:
	.type	.Lkeyboard_thread$local,@function
# %bb.0:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$65560, %rsp                    # imm = 0x10018
	#APP
	movq	%gs:0, %rax
	#NO_APP
	movq	%rax, 8(%rsp)                   # 8-byte Spill
	xorl	%r14d, %r14d
	leaq	keyboard_buf(%rip), %rbp
	.p2align	4, 0x90
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_6 Depth 2
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_32 Depth 3
	#APP
	leaq	.Lwake_up(%rip), %rax
	#NO_APP
	movq	8(%rsp), %rcx                   # 8-byte Reload
	#APP
	movq	%r14, (%rsp)                    # 8-byte Spill
	pushfq
	cli
	#NO_APP
	#MEMBARRIER
	#APP
	movq	%rsp, 16(%rcx)
	#NO_APP
	movq	%rax, 24(%rcx)
	xorl	%eax, %eax
	lock		cmpxchgq	%rcx, keyboard_sleeping_thread(%rip)
	jne	.LBB0_3
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movq	40(%rcx), %rdi
	andq	$-2, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
.Lwake_up:
	popfq
	#NO_APP
	movq	(%rsp), %r14                    # 8-byte Reload
	leaq	keyboard_buf(%rip), %rbp
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_6
	.p2align	4, 0x90
.LBB0_3:                                #   in Loop: Header=BB0_1 Depth=1
	#APP
	sti
	addq	$8, %rsp
	#NO_APP
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_6
	.p2align	4, 0x90
.LBB0_5:                                #   in Loop: Header=BB0_6 Depth=2
	#APP
	pause
	#NO_APP
.LBB0_6:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_32 Depth 3
	movzbl	(%r14,%rbp), %eax
	testb	%al, %al
	je	.LBB0_5
# %bb.7:                                #   in Loop: Header=BB0_6 Depth=2
	movb	%al, 16(%rsp)
	movb	$0, (%r14,%rbp)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	movl	$1, %r13d
	movl	$2, %eax
	leal	1(%r14), %ecx
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	cmpq	$1, %r15
	je	.LBB0_17
	.p2align	4, 0x90
.LBB0_9:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_6 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	(%rcx,%rbp), %edx
	testb	%dl, %dl
	je	.LBB0_18
# %bb.10:                               #   in Loop: Header=BB0_9 Depth=3
	movb	$0, (%rcx,%rbp)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	addl	$2, %r14d
	andl	$2097151, %r14d                 # imm = 0x1FFFFF
	movb	%dl, 15(%rsp,%rax)
	cmpq	$65536, %rax                    # imm = 0x10000
	je	.LBB0_11
# %bb.12:                               #   in Loop: Header=BB0_9 Depth=3
	cmpq	$1, %r15
	je	.LBB0_13
# %bb.14:                               #   in Loop: Header=BB0_9 Depth=3
	movzbl	(%r14,%rbp), %ecx
	testb	%cl, %cl
	je	.LBB0_15
# %bb.16:                               #   in Loop: Header=BB0_9 Depth=3
	movb	$0, (%r14,%rbp)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	addq	$2, %r13
	movb	%cl, 16(%rsp,%rax)
	addq	$2, %rax
	leal	1(%r14), %ecx
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	cmpq	$1, %r15
	jne	.LBB0_9
.LBB0_17:                               #   in Loop: Header=BB0_6 Depth=2
	decq	%rax
	movl	$1, %r15d
	movq	%rax, %r13
.LBB0_18:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%rcx, %r14
	jmp	.LBB0_19
.LBB0_11:                               #   in Loop: Header=BB0_6 Depth=2
	movl	$65536, %r13d                   # imm = 0x10000
	jmp	.LBB0_19
.LBB0_13:                               #   in Loop: Header=BB0_6 Depth=2
	incq	%r13
	movl	$1, %r15d
	jmp	.LBB0_19
.LBB0_15:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%rax, %r13
.LBB0_19:                               #   in Loop: Header=BB0_6 Depth=2
	movq	current_tty(%rip), %r12
	leaq	2097184(%r12), %rbx
	movq	%rbx, %rdi
	vzeroupper
	callq	mtx_lock@PLT
	testl	%eax, %eax
	jne	.LBB0_35
# %bb.20:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%r14, (%rsp)                    # 8-byte Spill
	movq	2097168(%r12), %rax
	movl	$2097152, %r14d                 # imm = 0x200000
	subq	%rax, %r14
	cmpq	%r14, %r13
	cmovbq	%r13, %r14
	testq	%r14, %r14
	je	.LBB0_33
# %bb.21:                               #   in Loop: Header=BB0_6 Depth=2
	leaq	(%r14,%rax), %r13
	cmpq	$1048576, %r13                  # imm = 0x100000
	movl	$0, %ebp
	cmovaeq	%r13, %rbp
	movq	2097152(%r12), %rcx
	cmpq	$1, %r14
	jne	.LBB0_31
# %bb.22:                               #   in Loop: Header=BB0_6 Depth=2
	xorl	%edx, %edx
	jmp	.LBB0_23
.LBB0_33:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	jne	.LBB0_35
# %bb.34:                               #   in Loop: Header=BB0_6 Depth=2
	cmpq	$1, %r15
	movq	(%rsp), %r14                    # 8-byte Reload
	jne	.LBB0_6
	jmp	.LBB0_1
.LBB0_31:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%r14, %rsi
	andq	$-2, %rsi
	leaq	1(%rax), %rdi
	xorl	%edx, %edx
	.p2align	4, 0x90
.LBB0_32:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_6 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	16(%rsp,%rdx), %r8d
	leal	1(%rcx), %r9d
	movb	%r8b, (%r12,%rcx)
	andl	$2097151, %r9d                  # imm = 0x1FFFFF
	leaq	(%rax,%rdx), %r8
	cmpq	%rbp, %r8
	cmovbeq	%rbp, %r8
	cmpb	$10, 16(%rsp,%rdx)
	cmovneq	%rbp, %r8
	addl	$2, %ecx
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	leaq	(%rdi,%rdx), %rbp
	cmpq	%r8, %rbp
	cmovbeq	%r8, %rbp
	movzbl	17(%rsp,%rdx), %r10d
	movb	%r10b, (%r12,%r9)
	cmpb	$10, 17(%rsp,%rdx)
	cmovneq	%r8, %rbp
	addq	$2, %rdx
	cmpq	%rdx, %rsi
	jne	.LBB0_32
.LBB0_23:                               #   in Loop: Header=BB0_6 Depth=2
	testb	$1, %r14b
	je	.LBB0_25
# %bb.24:                               #   in Loop: Header=BB0_6 Depth=2
	leal	1(%rcx), %esi
	andl	$2097151, %esi                  # imm = 0x1FFFFF
	addq	%rdx, %rax
	cmpq	%rbp, %rax
	cmovbeq	%rbp, %rax
	movzbl	16(%rsp,%rdx), %edi
	movb	%dil, (%r12,%rcx)
	cmpb	$10, 16(%rsp,%rdx)
	cmoveq	%rax, %rbp
	movq	%rsi, %rcx
.LBB0_25:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%rcx, 2097152(%r12)
	testq	%rbp, %rbp
	je	.LBB0_29
# %bb.26:                               #   in Loop: Header=BB0_6 Depth=2
	cmpq	$0, 2097176(%r12)
	jne	.LBB0_28
# %bb.27:                               #   in Loop: Header=BB0_6 Depth=2
	leaq	2097216(%r12), %rdi
	callq	cnd_broadcast@PLT
	testl	%eax, %eax
	jne	.LBB0_35
.LBB0_28:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%rbp, 2097176(%r12)
.LBB0_29:                               #   in Loop: Header=BB0_6 Depth=2
	movq	%r13, 2097168(%r12)
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	leaq	keyboard_buf(%rip), %rbp
	jne	.LBB0_35
# %bb.30:                               #   in Loop: Header=BB0_6 Depth=2
	xorl	%edi, %edi
	leaq	16(%rsp), %rsi
	movq	%r14, %rdx
	callq	*2097232(%r12)
	cmpq	$1, %r15
	movq	(%rsp), %r14                    # 8-byte Reload
	jne	.LBB0_6
	jmp	.LBB0_1
.LBB0_35:
	callq	abort@PLT
.Lfunc_end0:
	.size	keyboard_thread, .Lfunc_end0-keyboard_thread
	.size	.Lkeyboard_thread$local, .Lfunc_end0-keyboard_thread
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
