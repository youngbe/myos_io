	.text
	.file	"keyboard_thread_c.c"
	.section	.text.keyboard_thread,"ax",@progbits
	.globl	keyboard_thread                 # -- Begin function keyboard_thread
	.p2align	4, 0x90
	.type	keyboard_thread,@function
keyboard_thread:                        # @keyboard_thread
.Lkeyboard_thread$local:
# %bb.0:
	pushq	%rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$24, %rsp
	#APP
	movq	%gs:0, %r12
	#NO_APP
	xorl	%ebx, %ebx
	leaq	keyboard_buf(%rip), %rbp
	movq	%r12, 8(%rsp)                   # 8-byte Spill
	.p2align	4, 0x90
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_5 Depth 2
                                        #       Child Loop BB0_6 Depth 3
	#APP
	pushfq
	cli
	#NO_APP
	#APP
	movq	%rsp, 16(%r12)
	#NO_APP
	#APP
	leaq	.Lwake_up(%rip), %rax
	#NO_APP
	movq	%rax, 24(%r12)
	xorl	%eax, %eax
	lock		cmpxchgq	%r12, keyboard_sleeping_thread(%rip)
	jne	.LBB0_3
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movq	40(%r12), %rdi
	andq	$-2, %rdi
	movq	%rbx, 16(%rsp)                  # 8-byte Spill
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
.Lwake_up:
	popfq
	#NO_APP
	leaq	keyboard_buf(%rip), %rbp
	movq	16(%rsp), %rbx                  # 8-byte Reload
	movq	8(%rsp), %r12                   # 8-byte Reload
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
	movzwl	%bx, %ebx
	movzbl	(%rbx,%rbp), %eax
	movb	%al, 7(%rsp)
	testb	%al, %al
	jne	.LBB0_8
	.p2align	4, 0x90
.LBB0_6:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	#APP
	pause
	#NO_APP
	movzbl	(%rbx,%rbp), %eax
	movb	%al, 7(%rsp)
	testb	%al, %al
	je	.LBB0_6
.LBB0_8:                                #   in Loop: Header=BB0_5 Depth=2
	movb	$0, (%rbx,%rbp)
	movq	$-1, %r15
	lock		xaddq	%r15, keyboard_buf_used(%rip)
	movq	current_tty(%rip), %r13
	movl	$1, %edx
	xorl	%edi, %edi
	leaq	7(%rsp), %rsi
	vzeroupper
	callq	*2097232(%r13)
	leaq	2097184(%r13), %r14
	movq	%r14, %rdi
	callq	mtx_lock@PLT
	testl	%eax, %eax
	jne	.LBB0_18
# %bb.9:                                #   in Loop: Header=BB0_5 Depth=2
	cmpq	$2097152, 2097168(%r13)         # imm = 0x200000
	je	.LBB0_16
# %bb.10:                               #   in Loop: Header=BB0_5 Depth=2
	movzbl	7(%rsp), %eax
	movq	2097152(%r13), %rcx
	movb	%al, (%r13,%rcx)
	movl	2097152(%r13), %eax
	incl	%eax
	andl	$2097151, %eax                  # imm = 0x1FFFFF
	movq	%rax, 2097152(%r13)
	movq	2097168(%r13), %rax
	incq	%rax
	movq	%rax, 2097168(%r13)
	cmpq	$1048575, %rax                  # imm = 0xFFFFF
	ja	.LBB0_12
# %bb.11:                               #   in Loop: Header=BB0_5 Depth=2
	cmpb	$10, 7(%rsp)
	jne	.LBB0_16
.LBB0_12:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$0, 2097176(%r13)
	jne	.LBB0_15
# %bb.13:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	2097216(%r13), %rdi
	callq	cnd_broadcast@PLT
	testl	%eax, %eax
	jne	.LBB0_18
# %bb.14:                               #   in Loop: Header=BB0_5 Depth=2
	movq	2097168(%r13), %rax
.LBB0_15:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, 2097176(%r13)
.LBB0_16:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r14, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	jne	.LBB0_18
# %bb.17:                               #   in Loop: Header=BB0_5 Depth=2
	incl	%ebx
	cmpq	$1, %r15
	jne	.LBB0_5
	jmp	.LBB0_1
.LBB0_18:
	callq	abort@PLT
.Lfunc_end0:
	.size	keyboard_thread, .Lfunc_end0-keyboard_thread
                                        # -- End function
	.ident	"Ubuntu clang version 15.0.7"
	.section	".note.GNU-stack","",@progbits
