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
	movq	%gs:0, %rbp
	#NO_APP
	leaq	keyboard_buf(%rip), %r13
	movq	%rbp, 16(%rsp)                  # 8-byte Spill
	.p2align	4, 0x90
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_5 Depth 2
                                        #       Child Loop BB0_6 Depth 3
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_29 Depth 3
	#APP
	leaq	.Lwake_up(%rip), %rax
	#NO_APP
	#APP
	pushfq
	cli
	#NO_APP
	#MEMBARRIER
	#APP
	movq	%rsp, 16(%rbp)
	#NO_APP
	movq	%rax, 24(%rbp)
	xorl	%eax, %eax
	lock		cmpxchgq	%rbp, keyboard_sleeping_thread(%rip)
	jne	.LBB0_3
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movq	40(%rbp), %rdi
	andq	$-2, %rdi
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
.Lwake_up:
	popfq
	#NO_APP
	leaq	keyboard_buf(%rip), %r13
	movq	16(%rsp), %rbp                  # 8-byte Reload
	jmp	.LBB0_4
	.p2align	4, 0x90
.LBB0_3:                                #   in Loop: Header=BB0_1 Depth=1
	#APP
	sti
	addq	$8, %rsp
	#NO_APP
.LBB0_4:                                #   in Loop: Header=BB0_1 Depth=1
	movq	$0, keyboard_sleeping_thread(%rip)
	.p2align	4, 0x90
.LBB0_5:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_6 Depth 3
                                        #       Child Loop BB0_9 Depth 3
                                        #       Child Loop BB0_29 Depth 3
	movq	(%rsp), %rcx                    # 8-byte Reload
	movzbl	(%rcx,%r13), %eax
	testb	%al, %al
	jne	.LBB0_8
	.p2align	4, 0x90
.LBB0_6:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	#APP
	pause
	#NO_APP
	movzbl	(%rcx,%r13), %eax
	testb	%al, %al
	je	.LBB0_6
.LBB0_8:                                #   in Loop: Header=BB0_5 Depth=2
	movb	%al, 32(%rsp)
	movb	$0, (%rcx,%r13)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	leal	1(%rcx), %eax
	andl	$2097151, %eax                  # imm = 0x1FFFFF
	movl	$1, %r14d
	movq	%rax, %rcx
	jmp	.LBB0_9
	.p2align	4, 0x90
.LBB0_13:                               #   in Loop: Header=BB0_9 Depth=3
	movb	$0, (%rcx,%r13)
	incl	%ecx
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
                                        # kill: def $ecx killed $ecx killed $rcx def $rcx
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	movb	%al, 32(%rsp,%r14)
	incq	%r14
	cmpq	$65536, %r14                    # imm = 0x10000
	je	.LBB0_14
.LBB0_9:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	(%rcx,%r13), %eax
	testb	%al, %al
	je	.LBB0_10
# %bb.11:                               #   in Loop: Header=BB0_9 Depth=3
	cmpq	$1, %r15
	jne	.LBB0_13
# %bb.12:                               #   in Loop: Header=BB0_9 Depth=3
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_13
	.p2align	4, 0x90
.LBB0_10:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rcx, (%rsp)                    # 8-byte Spill
	jmp	.LBB0_15
	.p2align	4, 0x90
.LBB0_14:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rcx, (%rsp)                    # 8-byte Spill
	movl	$65536, %r14d                   # imm = 0x10000
.LBB0_15:                               #   in Loop: Header=BB0_5 Depth=2
	movq	current_tty(%rip), %r12
	xorl	%edi, %edi
	leaq	32(%rsp), %rsi
	movq	%r14, %rdx
	vzeroupper
	callq	*2097232(%r12)
	leaq	2097184(%r12), %rbx
	movq	%rbx, %rdi
	callq	mtx_lock@PLT
	testl	%eax, %eax
	jne	.LBB0_34
# %bb.16:                               #   in Loop: Header=BB0_5 Depth=2
	movq	2097168(%r12), %rax
	movl	$2097152, %edx                  # imm = 0x200000
	subq	%rax, %rdx
	cmpq	%rdx, %r14
	cmovbq	%r14, %rdx
	testq	%rdx, %rdx
	je	.LBB0_26
# %bb.17:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	(%rdx,%rax), %r14
	cmpq	$1048576, %r14                  # imm = 0x100000
	movl	$0, %r13d
	cmovaeq	%r14, %r13
	movq	2097152(%r12), %rcx
	cmpq	$1, %rdx
	jne	.LBB0_28
# %bb.18:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%esi, %esi
.LBB0_19:                               #   in Loop: Header=BB0_5 Depth=2
	testb	$1, %dl
	je	.LBB0_21
# %bb.20:                               #   in Loop: Header=BB0_5 Depth=2
	leal	1(%rcx), %edx
	andl	$2097151, %edx                  # imm = 0x1FFFFF
	addq	%rsi, %rax
	incq	%rax
	cmpq	%r13, %rax
	cmovbeq	%r13, %rax
	movzbl	32(%rsp,%rsi), %edi
	movb	%dil, (%r12,%rcx)
	cmpb	$10, 32(%rsp,%rsi)
	cmoveq	%rax, %r13
	movq	%rdx, %rcx
.LBB0_21:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rcx, 2097152(%r12)
	testq	%r13, %r13
	je	.LBB0_25
# %bb.22:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$0, 2097176(%r12)
	jne	.LBB0_24
# %bb.23:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	2097216(%r12), %rdi
	callq	cnd_broadcast@PLT
	testl	%eax, %eax
	jne	.LBB0_34
.LBB0_24:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r13, 2097176(%r12)
.LBB0_25:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r14, 2097168(%r12)
	leaq	keyboard_buf(%rip), %r13
.LBB0_26:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	jne	.LBB0_34
# %bb.27:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$1, %r15
	jne	.LBB0_5
	jmp	.LBB0_1
	.p2align	4, 0x90
.LBB0_28:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rdx, %rdi
	movabsq	$9223372036854775806, %rsi      # imm = 0x7FFFFFFFFFFFFFFE
	andq	%rsi, %rdi
	leaq	1(%rax), %r8
	xorl	%esi, %esi
	jmp	.LBB0_29
	.p2align	4, 0x90
.LBB0_33:                               #   in Loop: Header=BB0_29 Depth=3
	cmpb	$10, %r9b
	cmoveq	%r10, %r13
	addq	$2, %r8
	cmpq	%rsi, %rdi
	je	.LBB0_19
.LBB0_29:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	32(%rsp,%rsi), %r10d
	leal	1(%rcx), %r9d
	movb	%r10b, (%r12,%rcx)
	andl	$2097151, %r9d                  # imm = 0x1FFFFF
	movq	%r8, %r10
	cmpq	%r13, %r8
	ja	.LBB0_31
# %bb.30:                               #   in Loop: Header=BB0_29 Depth=3
	movq	%r13, %r10
.LBB0_31:                               #   in Loop: Header=BB0_29 Depth=3
	cmpb	$10, 32(%rsp,%rsi)
	cmoveq	%r10, %r13
	movzbl	33(%rsp,%rsi), %r10d
	addl	$2, %ecx
	movb	%r10b, (%r12,%r9)
	andl	$2097151, %ecx                  # imm = 0x1FFFFF
	movzbl	33(%rsp,%rsi), %r9d
	addq	$2, %rsi
	leaq	1(%r8), %r10
	cmpq	%r13, %r10
	ja	.LBB0_33
# %bb.32:                               #   in Loop: Header=BB0_29 Depth=3
	movq	%r13, %r10
	jmp	.LBB0_33
.LBB0_34:
	callq	abort@PLT
.Lfunc_end0:
	.size	keyboard_thread, .Lfunc_end0-keyboard_thread
	.size	.Lkeyboard_thread$local, .Lfunc_end0-keyboard_thread
                                        # -- End function
	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
