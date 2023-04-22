	.text
	.file	"keyboard_thread_c.c"
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
	subq	$65576, %rsp                    # imm = 0x10028
	#APP
	rdfsbaseq	%rax
	#NO_APP
	#APP
	stmxcsr	28(%rsp)
	#NO_APP
	#APP
	wait
	fnstcw	14(%rsp)
	#NO_APP
	xorl	%eax, %eax
	movq	%rax, (%rsp)                    # 8-byte Spill
	#APP
	movq	%gs:0, %rax
	#NO_APP
	movq	%rax, 16(%rsp)                  # 8-byte Spill
	leaq	keyboard_buf(%rip), %rbp
	.p2align	4, 0x90
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_6 Depth 2
                                        #     Child Loop BB0_8 Depth 2
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_28 Depth 3
	#APP
	leaq	.Lwake_up(%rip), %rax
	#NO_APP
	movq	16(%rsp), %rcx                  # 8-byte Reload
	#APP
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
	leaq	keyboard_buf(%rip), %rbp
	jmp	.LBB0_4
	.p2align	4, 0x90
.LBB0_3:                                #   in Loop: Header=BB0_1 Depth=1
	#APP
	sti
	addq	$8, %rsp
	#NO_APP
.LBB0_4:                                #   in Loop: Header=BB0_1 Depth=1
	movq	$0, keyboard_sleeping_thread(%rip)
	movq	(%rsp), %rcx                    # 8-byte Reload
	movzbl	(%rcx,%rbp), %eax
	testb	%al, %al
	je	.LBB0_6
	jmp	.LBB0_8
	.p2align	4, 0x90
.LBB0_33:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	jne	.LBB0_35
# %bb.34:                               #   in Loop: Header=BB0_8 Depth=2
	cmpq	$1, %r15
	je	.LBB0_1
.LBB0_5:                                #   in Loop: Header=BB0_8 Depth=2
	movq	(%rsp), %rcx                    # 8-byte Reload
	movzbl	(%rcx,%rbp), %eax
	testb	%al, %al
	jne	.LBB0_8
	.p2align	4, 0x90
.LBB0_6:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	#APP
	pause
	#NO_APP
	movzbl	(%rcx,%rbp), %eax
	testb	%al, %al
	je	.LBB0_6
.LBB0_8:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_28 Depth 3
	movb	%al, 32(%rsp)
	movb	$0, (%rcx,%rbp)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	leal	1(%rcx), %eax
	andl	$2097151, %eax                  # imm = 0x1FFFFF
	movl	$1, %r13d
	movq	%rax, %rcx
	jmp	.LBB0_9
	.p2align	4, 0x90
.LBB0_13:                               #   in Loop: Header=BB0_9 Depth=3
	movb	$0, (%rcx,%rbp)
	incl	%ecx
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
                                        # kill: def $ecx killed $ecx killed $rcx def $rcx
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	movb	%al, 32(%rsp,%r13)
	incq	%r13
	cmpq	$65536, %r13                    # imm = 0x10000
	je	.LBB0_14
.LBB0_9:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_8 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	(%rcx,%rbp), %eax
	testb	%al, %al
	je	.LBB0_10
# %bb.11:                               #   in Loop: Header=BB0_9 Depth=3
	cmpq	$1, %r15
	jne	.LBB0_13
# %bb.12:                               #   in Loop: Header=BB0_9 Depth=3
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_13
	.p2align	4, 0x90
.LBB0_10:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%rcx, (%rsp)                    # 8-byte Spill
	jmp	.LBB0_15
	.p2align	4, 0x90
.LBB0_14:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%rcx, (%rsp)                    # 8-byte Spill
	movl	$65536, %r13d                   # imm = 0x10000
.LBB0_15:                               #   in Loop: Header=BB0_8 Depth=2
	movq	current_tty(%rip), %r12
	leaq	2097184(%r12), %rbx
	movq	%rbx, %rdi
	vzeroupper
	callq	mtx_lock@PLT
	testl	%eax, %eax
	jne	.LBB0_35
# %bb.16:                               #   in Loop: Header=BB0_8 Depth=2
	movq	2097168(%r12), %rax
	movl	$2097152, %r14d                 # imm = 0x200000
	subq	%rax, %r14
	cmpq	%r14, %r13
	cmovbq	%r13, %r14
	testq	%r14, %r14
	je	.LBB0_33
# %bb.17:                               #   in Loop: Header=BB0_8 Depth=2
	leaq	(%r14,%rax), %r13
	cmpq	$1048576, %r13                  # imm = 0x100000
	movl	$0, %ebp
	cmovaeq	%r13, %rbp
	movq	2097152(%r12), %rcx
	cmpq	$1, %r14
	jne	.LBB0_27
# %bb.18:                               #   in Loop: Header=BB0_8 Depth=2
	xorl	%edx, %edx
.LBB0_19:                               #   in Loop: Header=BB0_8 Depth=2
	testb	$1, %r14b
	je	.LBB0_21
# %bb.20:                               #   in Loop: Header=BB0_8 Depth=2
	leal	1(%rcx), %esi
	andl	$2097151, %esi                  # imm = 0x1FFFFF
	addq	%rdx, %rax
	incq	%rax
	cmpq	%rbp, %rax
	cmovbeq	%rbp, %rax
	movzbl	32(%rsp,%rdx), %edi
	movb	%dil, (%r12,%rcx)
	cmpb	$10, 32(%rsp,%rdx)
	cmoveq	%rax, %rbp
	movq	%rsi, %rcx
.LBB0_21:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%rcx, 2097152(%r12)
	testq	%rbp, %rbp
	je	.LBB0_25
# %bb.22:                               #   in Loop: Header=BB0_8 Depth=2
	cmpq	$0, 2097176(%r12)
	jne	.LBB0_24
# %bb.23:                               #   in Loop: Header=BB0_8 Depth=2
	leaq	2097216(%r12), %rdi
	callq	cnd_broadcast@PLT
	testl	%eax, %eax
	jne	.LBB0_35
.LBB0_24:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%rbp, 2097176(%r12)
.LBB0_25:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%r13, 2097168(%r12)
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	leaq	keyboard_buf(%rip), %rbp
	jne	.LBB0_35
# %bb.26:                               #   in Loop: Header=BB0_8 Depth=2
	xorl	%edi, %edi
	leaq	32(%rsp), %rsi
	movq	%r14, %rdx
	callq	*2097232(%r12)
	cmpq	$1, %r15
	jne	.LBB0_5
	jmp	.LBB0_1
	.p2align	4, 0x90
.LBB0_27:                               #   in Loop: Header=BB0_8 Depth=2
	movq	%r14, %rsi
	movabsq	$9223372036854775806, %rdx      # imm = 0x7FFFFFFFFFFFFFFE
	andq	%rdx, %rsi
	leaq	1(%rax), %rdi
	xorl	%edx, %edx
	jmp	.LBB0_28
	.p2align	4, 0x90
.LBB0_32:                               #   in Loop: Header=BB0_28 Depth=3
	cmpb	$10, %r8b
	cmoveq	%r9, %rbp
	addq	$2, %rdi
	cmpq	%rdx, %rsi
	je	.LBB0_19
.LBB0_28:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_8 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	32(%rsp,%rdx), %r9d
	leal	1(%rcx), %r8d
	movb	%r9b, (%r12,%rcx)
	andl	$2097151, %r8d                  # imm = 0x1FFFFF
	movq	%rdi, %r9
	cmpq	%rbp, %rdi
	ja	.LBB0_30
# %bb.29:                               #   in Loop: Header=BB0_28 Depth=3
	movq	%rbp, %r9
.LBB0_30:                               #   in Loop: Header=BB0_28 Depth=3
	cmpb	$10, 32(%rsp,%rdx)
	cmoveq	%r9, %rbp
	movzbl	33(%rsp,%rdx), %r9d
	addl	$2, %ecx
	movb	%r9b, (%r12,%r8)
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	movzbl	33(%rsp,%rdx), %r8d
	addq	$2, %rdx
	leaq	1(%rdi), %r9
	cmpq	%rbp, %r9
	ja	.LBB0_32
# %bb.31:                               #   in Loop: Header=BB0_28 Depth=3
	movq	%rbp, %r9
	jmp	.LBB0_32
.LBB0_35:
	callq	abort@PLT
.Lfunc_end0:
	.size	keyboard_thread, .Lfunc_end0-keyboard_thread
	.size	.Lkeyboard_thread$local, .Lfunc_end0-keyboard_thread
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
