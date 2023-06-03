	.text
	.file	"keyboard_thread_c.c"
	.section	.rodata.cst32,"aM",@progbits,32
	.p2align	5, 0x0                          # -- Begin function keyboard_thread
.LCPI0_0:
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
.LCPI0_1:
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
.LCPI0_2:
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0
.LCPI0_3:
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.byte	8                               # 0x8
	.byte	32                              # 0x20
	.byte	8                               # 0x8
	.section	.text.keyboard_thread,"ax",@progbits
	.globl	keyboard_thread
	.p2align	4, 0x90
	.type	keyboard_thread,@function
keyboard_thread:                        # @keyboard_thread
.Lkeyboard_thread$local:
	.type	.Lkeyboard_thread$local,@function
# %bb.0:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$196696, %rsp                   # imm = 0x30058
	#APP
	rdfsbaseq	%rax
	#NO_APP
	#APP
	stmxcsr	-92(%rbp)
	#NO_APP
	#APP
	wait
	fnstcw	-58(%rbp)
	#NO_APP
	xorl	%r15d, %r15d
	#APP
	movq	%gs:0, %rsi
	#NO_APP
	leaq	keyboard_buf(%rip), %r11
	leaq	to_ascii_cap(%rip), %r12
	leaq	to_ascii_cap_shift(%rip), %r13
	movq	%rsi, -72(%rbp)                 # 8-byte Spill
	.p2align	4, 0x90
.LBB0_1:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_5 Depth 2
                                        #       Child Loop BB0_8 Depth 3
                                        #       Child Loop BB0_29 Depth 3
                                        #       Child Loop BB0_69 Depth 3
                                        #       Child Loop BB0_73 Depth 3
                                        #       Child Loop BB0_22 Depth 3
                                        #       Child Loop BB0_64 Depth 3
	#APP
	leaq	.Lwake_up(%rip), %rax
	#NO_APP
	#APP
	pushfq
	cli
	#NO_APP
	#APP
	movq	%rsp, 16(%rsi)
	#NO_APP
	movq	%rax, 24(%rsi)
	xorl	%eax, %eax
	lock		cmpxchgq	%rsi, keyboard_sleeping_thread(%rip)
	jne	.LBB0_3
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movq	40(%rsi), %rdi
	andq	$-2, %rdi
	movq	%r15, -56(%rbp)                 # 8-byte Spill
	#APP
	rdgsbaseq	%rsp
	addq	$65520, %rsp                    # imm = 0xFFF0
	jmp	switch_to_empty
.Lwake_up:
	popfq
	#NO_APP
	leaq	to_ascii_cap_shift(%rip), %r13
	leaq	to_ascii_cap(%rip), %r12
	movq	-56(%rbp), %r15                 # 8-byte Reload
	leaq	keyboard_buf(%rip), %r11
	movq	-72(%rbp), %rsi                 # 8-byte Reload
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_5
	.p2align	4, 0x90
.LBB0_3:                                #   in Loop: Header=BB0_1 Depth=1
	#APP
	sti
	addq	$8, %rsp
	#NO_APP
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_5
	.p2align	4, 0x90
.LBB0_4:                                #   in Loop: Header=BB0_5 Depth=2
	#APP
	pause
	#NO_APP
.LBB0_5:                                #   Parent Loop BB0_1 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_8 Depth 3
                                        #       Child Loop BB0_29 Depth 3
                                        #       Child Loop BB0_69 Depth 3
                                        #       Child Loop BB0_73 Depth 3
                                        #       Child Loop BB0_22 Depth 3
                                        #       Child Loop BB0_64 Depth 3
	movzwl	(%r11,%r15,2), %eax
	testw	%ax, %ax
	je	.LBB0_4
# %bb.6:                                #   in Loop: Header=BB0_5 Depth=2
	movw	%ax, -196736(%rbp)
	movw	$0, (%r11,%r15,2)
	movl	$-1, %ecx
	lock		xaddl	%ecx, keyboard_buf_used(%rip)
	leal	1(%r15), %edx
	andl	$2097151, %edx                  # imm = 0x1FFFFF
	movl	$1, %eax
	movq	%rdx, %r15
	jmp	.LBB0_8
	.p2align	4, 0x90
.LBB0_7:                                #   in Loop: Header=BB0_8 Depth=3
	movw	$0, (%r11,%r15,2)
	leal	1(%r15), %edi
	movl	$-1, %ecx
	lock		xaddl	%ecx, keyboard_buf_used(%rip)
	movl	%edi, %r15d
	andl	$2097151, %r15d                 # imm = 0x1FFFFF
	movw	%dx, -196736(%rbp,%rax,2)
	incq	%rax
	cmpq	$65536, %rax                    # imm = 0x10000
	je	.LBB0_25
.LBB0_8:                                #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzwl	(%r11,%r15,2), %edx
	testw	%dx, %dx
	je	.LBB0_11
# %bb.9:                                #   in Loop: Header=BB0_8 Depth=3
	cmpl	$1, %ecx
	jne	.LBB0_7
# %bb.10:                               #   in Loop: Header=BB0_8 Depth=3
	movq	$0, keyboard_sleeping_thread(%rip)
	jmp	.LBB0_7
.LBB0_11:                               #   in Loop: Header=BB0_5 Depth=2
	movl	%ecx, %ecx
	movq	%rcx, -88(%rbp)                 # 8-byte Spill
	testq	%rax, %rax
	jne	.LBB0_26
# %bb.12:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%eax, %eax
	movq	%rax, -48(%rbp)                 # 8-byte Spill
	xorl	%r14d, %r14d
.LBB0_13:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r14, %rax
	orq	-48(%rbp), %rax                 # 8-byte Folded Reload
	je	.LBB0_61
# %bb.14:                               #   in Loop: Header=BB0_5 Depth=2
	movq	current_tty(%rip), %r12
	leaq	2097184(%r12), %rbx
	movq	%rbx, %rdi
	vzeroupper
	callq	mtx_lock@PLT
	testl	%eax, %eax
	jne	.LBB0_80
# %bb.15:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r15, -56(%rbp)                 # 8-byte Spill
	movq	2097168(%r12), %r15
	movq	2097176(%r12), %rax
	movq	%r15, %r13
	subq	%rax, %r13
	movq	-48(%rbp), %rcx                 # 8-byte Reload
	cmpq	%rcx, %r13
	cmovaeq	%rcx, %r13
	testq	%r13, %r13
	je	.LBB0_37
# %bb.16:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, -48(%rbp)                 # 8-byte Spill
	movq	%rsp, %r8
	movq	%rsp, %rsi
	leaq	15(,%r13,2), %rax
	addq	%r13, %rax
	andq	$-16, %rax
	subq	%rax, %rsi
	movq	%rsi, %rsp
	cmpq	$16, %r13
	jae	.LBB0_39
.LBB0_17:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%eax, %eax
.LBB0_18:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, %rcx
	orq	$1, %rcx
	testb	$1, %r13b
	je	.LBB0_20
# %bb.19:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	(%rax,%rax,2), %rax
	movb	$8, (%rsi,%rax)
	movq	%rax, %rdx
	orq	$1, %rdx
	movb	$32, (%rsi,%rdx)
	orq	$2, %rax
	movb	$8, (%rsi,%rax)
	movq	%rcx, %rax
.LBB0_20:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	%rcx, %r13
	je	.LBB0_23
# %bb.21:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	(%rax,%rax,2), %rcx
	addq	%rsi, %rcx
	addq	$5, %rcx
	movq	%r13, %rdx
	subq	%rax, %rdx
	.p2align	4, 0x90
.LBB0_22:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movl	$134750216, -5(%rcx)            # imm = 0x8082008
	movw	$2080, -1(%rcx)                 # imm = 0x820
	addq	$6, %rcx
	addq	$-2, %rdx
	jne	.LBB0_22
.LBB0_23:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	(,%r13,2), %rdx
	addq	%r13, %rdx
	testq	%r14, %r14
	movq	%rbx, -80(%rbp)                 # 8-byte Spill
	je	.LBB0_47
# %bb.24:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rsi, -128(%rbp)
	movq	%rdx, -120(%rbp)
	leaq	-65664(%rbp), %rax
	movq	%rax, -112(%rbp)
	movq	%r14, -104(%rbp)
	xorl	%edi, %edi
	leaq	-128(%rbp), %rsi
	movl	$2, %edx
	movq	%r8, %rbx
	vzeroupper
	callq	*2097248(%r12)
	jmp	.LBB0_48
.LBB0_25:                               #   in Loop: Header=BB0_5 Depth=2
	movl	%ecx, %eax
	movq	%rax, -88(%rbp)                 # 8-byte Spill
	movl	$65536, %eax                    # imm = 0x10000
.LBB0_26:                               #   in Loop: Header=BB0_5 Depth=2
	movzbl	keyboard_thread.is_cap(%rip), %ecx
	movzbl	keymap+77(%rip), %r8d
	xorl	%r14d, %r14d
	xorl	%edx, %edx
	movq	%rdx, -48(%rbp)                 # 8-byte Spill
	xorl	%edx, %edx
	jmp	.LBB0_29
.LBB0_27:                               #   in Loop: Header=BB0_29 Depth=3
	cmpq	$1, %r14
	adcq	$0, -48(%rbp)                   # 8-byte Folded Spill
	subq	$1, %r14
	movl	$0, %edi
	cmovbq	%rdi, %r14
	.p2align	4, 0x90
.LBB0_28:                               #   in Loop: Header=BB0_29 Depth=3
	incq	%rdx
	movl	%r10d, %r8d
	cmpq	%rdx, %rax
	je	.LBB0_13
.LBB0_29:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzwl	-196736(%rbp,%rdx,2), %r10d
	movq	%r10, %rdi
	shrq	%rdi
	andl	$1, %r10d
	cmpw	$77, %di
	jne	.LBB0_33
# %bb.30:                               #   in Loop: Header=BB0_29 Depth=3
	testw	%r10w, %r10w
	je	.LBB0_33
# %bb.31:                               #   in Loop: Header=BB0_29 Depth=3
	testb	%r8b, %r8b
	jne	.LBB0_33
# %bb.32:                               #   in Loop: Header=BB0_29 Depth=3
	xorb	$1, %cl
	movb	%cl, keyboard_thread.is_cap(%rip)
	movb	%r10b, keymap+77(%rip)
	movl	$77, %edi
	jmp	.LBB0_35
	.p2align	4, 0x90
.LBB0_33:                               #   in Loop: Header=BB0_29 Depth=3
	leaq	keymap(%rip), %r8
	movb	%r10b, (%rdi,%r8)
	testw	%r10w, %r10w
	movzbl	keymap+77(%rip), %r10d
	je	.LBB0_28
# %bb.34:                               #   in Loop: Header=BB0_29 Depth=3
	cmpw	$72, %di
	je	.LBB0_27
	.p2align	4, 0x90
.LBB0_35:                               #   in Loop: Header=BB0_29 Depth=3
	movzbl	keymap+81(%rip), %r8d
	orb	keymap+85(%rip), %r8b
	movq	%r13, %r8
	cmoveq	%r12, %r8
	leaq	to_ascii_shift(%rip), %r9
	leaq	to_ascii(%rip), %rbx
	cmoveq	%rbx, %r9
	testb	%cl, %cl
	cmovneq	%r8, %r9
	movzbl	(%r9,%rdi), %edi
	testb	%dil, %dil
	je	.LBB0_28
# %bb.36:                               #   in Loop: Header=BB0_29 Depth=3
	movb	%dil, -65664(%rbp,%r14)
	incq	%r14
	jmp	.LBB0_28
.LBB0_37:                               #   in Loop: Header=BB0_5 Depth=2
	testq	%r14, %r14
	je	.LBB0_60
# %bb.38:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, -48(%rbp)                 # 8-byte Spill
	movq	%rbx, -80(%rbp)                 # 8-byte Spill
	xorl	%edi, %edi
	leaq	-65664(%rbp), %rsi
	movq	%r14, %rdx
	callq	*2097240(%r12)
	movq	2097152(%r12), %rbx
	jmp	.LBB0_50
.LBB0_39:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	-1(%r13), %rax
	leaq	1(%rsi), %rdi
	movl	$3, %ecx
	mulq	%rcx
	seto	%cl
	leaq	(%rsi,%rax), %rdx
	incq	%rdx
	cmpq	%rdi, %rdx
	vmovaps	.LCPI0_3(%rip), %xmm0           # xmm0 = [8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8]
	vmovaps	.LCPI0_2(%rip), %ymm1           # ymm1 = [8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32]
	vmovaps	.LCPI0_0(%rip), %ymm2           # ymm2 = [32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8]
	vmovaps	.LCPI0_1(%rip), %ymm3           # ymm3 = [8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8,32,8,8]
	jb	.LBB0_17
# %bb.40:                               #   in Loop: Header=BB0_5 Depth=2
	testb	%cl, %cl
	jne	.LBB0_17
# %bb.41:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	2(%rsi), %rdx
	leaq	(%rsi,%rax), %rdi
	addq	$2, %rdi
	cmpq	%rdx, %rdi
	jb	.LBB0_17
# %bb.42:                               #   in Loop: Header=BB0_5 Depth=2
	testb	%cl, %cl
	jne	.LBB0_17
# %bb.43:                               #   in Loop: Header=BB0_5 Depth=2
	addq	%rsi, %rax
	cmpq	%rsi, %rax
	jb	.LBB0_17
# %bb.44:                               #   in Loop: Header=BB0_5 Depth=2
	movl	$0, %eax
	testb	%cl, %cl
	jne	.LBB0_18
# %bb.45:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$32, %r13
	jae	.LBB0_68
# %bb.46:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%eax, %eax
	jmp	.LBB0_72
.LBB0_47:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%edi, %edi
	movq	%r8, %rbx
	vzeroupper
	callq	*2097240(%r12)
.LBB0_48:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rbx, %rax
	movq	%rbx, %rsp
	movq	2097152(%r12), %rax
	movq	%rax, %rbx
	subq	%r13, %rbx
	jae	.LBB0_50
# %bb.49:                               #   in Loop: Header=BB0_5 Depth=2
	subq	%r13, %rax
	addq	$2097152, %rax                  # imm = 0x200000
	movq	%rax, %rbx
.LBB0_50:                               #   in Loop: Header=BB0_5 Depth=2
	subq	%r13, %r15
	movl	$2097152, %eax                  # imm = 0x200000
	subq	%r15, %rax
	cmpq	%rax, %r14
	cmovbq	%r14, %rax
	leaq	(%rax,%r15), %r14
	testq	%rax, %rax
	je	.LBB0_59
# %bb.51:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$1048576, %r14                  # imm = 0x100000
	movl	$0, %r13d
	cmovaeq	%r14, %r13
	cmpq	$1, %rax
	jne	.LBB0_62
# %bb.52:                               #   in Loop: Header=BB0_5 Depth=2
	xorl	%ecx, %ecx
.LBB0_53:                               #   in Loop: Header=BB0_5 Depth=2
	testb	$1, %al
	je	.LBB0_55
# %bb.54:                               #   in Loop: Header=BB0_5 Depth=2
	leal	1(%rbx), %eax
	andl	$2097151, %eax                  # imm = 0x1FFFFF
	leaq	(%rcx,%r15), %rdx
	incq	%rdx
	cmpq	%r13, %rdx
	cmovbeq	%r13, %rdx
	movzbl	-65664(%rbp,%rcx), %esi
	movb	%sil, (%r12,%rbx)
	cmpb	$10, -65664(%rbp,%rcx)
	cmoveq	%rdx, %r13
	movq	%rax, %rbx
.LBB0_55:                               #   in Loop: Header=BB0_5 Depth=2
	testq	%r13, %r13
	je	.LBB0_59
# %bb.56:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	$0, -48(%rbp)                   # 8-byte Folded Reload
	jne	.LBB0_58
# %bb.57:                               #   in Loop: Header=BB0_5 Depth=2
	leaq	2097216(%r12), %rdi
	callq	cnd_broadcast@PLT
	testl	%eax, %eax
	jne	.LBB0_80
.LBB0_58:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r13, 2097176(%r12)
.LBB0_59:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rbx, 2097152(%r12)
	movq	%r14, 2097168(%r12)
	movq	-80(%rbp), %rbx                 # 8-byte Reload
.LBB0_60:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rbx, %rdi
	callq	mtx_unlock@PLT
	testl	%eax, %eax
	movq	-72(%rbp), %rsi                 # 8-byte Reload
	leaq	keyboard_buf(%rip), %r11
	movq	-56(%rbp), %r15                 # 8-byte Reload
	leaq	to_ascii_cap(%rip), %r12
	leaq	to_ascii_cap_shift(%rip), %r13
	jne	.LBB0_80
.LBB0_61:                               #   in Loop: Header=BB0_5 Depth=2
	cmpl	$1, -88(%rbp)                   # 4-byte Folded Reload
	jne	.LBB0_5
	jmp	.LBB0_1
.LBB0_62:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, %rdx
	andq	$-2, %rdx
	leaq	1(%r15), %rsi
	xorl	%ecx, %ecx
	jmp	.LBB0_64
	.p2align	4, 0x90
.LBB0_63:                               #   in Loop: Header=BB0_64 Depth=3
	cmpb	$10, %dil
	cmoveq	%r8, %r13
	addq	$2, %rsi
	cmpq	%rcx, %rdx
	je	.LBB0_53
.LBB0_64:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	movzbl	-65664(%rbp,%rcx), %r8d
	leal	1(%rbx), %edi
	movb	%r8b, (%r12,%rbx)
	andl	$2097151, %edi                  # imm = 0x1FFFFF
	movq	%rsi, %r8
	cmpq	%r13, %rsi
	ja	.LBB0_66
# %bb.65:                               #   in Loop: Header=BB0_64 Depth=3
	movq	%r13, %r8
.LBB0_66:                               #   in Loop: Header=BB0_64 Depth=3
	cmpb	$10, -65664(%rbp,%rcx)
	cmoveq	%r8, %r13
	movzbl	-65663(%rbp,%rcx), %r8d
	addl	$2, %ebx
	movb	%r8b, (%r12,%rdi)
	andl	$2097151, %ebx                  # imm = 0x1FFFFF
	movzbl	-65663(%rbp,%rcx), %edi
	addq	$2, %rcx
	leaq	1(%rsi), %r8
	cmpq	%r13, %r8
	ja	.LBB0_63
# %bb.67:                               #   in Loop: Header=BB0_64 Depth=3
	movq	%r13, %r8
	jmp	.LBB0_63
.LBB0_68:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%r13, %rax
	andq	$-32, %rax
	movq	%rax, %rcx
	movq	%rsi, %rdx
	.p2align	4, 0x90
.LBB0_69:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovups	%ymm2, 64(%rdx)
	vmovups	%ymm3, 32(%rdx)
	vmovups	%ymm1, (%rdx)
	addq	$96, %rdx
	addq	$-32, %rcx
	jne	.LBB0_69
# %bb.70:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	%rax, %r13
	je	.LBB0_23
# %bb.71:                               #   in Loop: Header=BB0_5 Depth=2
	testb	$16, %r13b
	je	.LBB0_18
.LBB0_72:                               #   in Loop: Header=BB0_5 Depth=2
	movq	%rax, %rcx
	movq	%r13, %rax
	andq	$-16, %rax
	leaq	(%rcx,%rcx,2), %rdx
	addq	%rsi, %rdx
	subq	%rax, %rcx
	.p2align	4, 0x90
.LBB0_73:                               #   Parent Loop BB0_1 Depth=1
                                        #     Parent Loop BB0_5 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	vmovaps	%xmm0, 32(%rdx)
	vmovups	%ymm1, (%rdx)
	addq	$48, %rdx
	addq	$16, %rcx
	jne	.LBB0_73
# %bb.74:                               #   in Loop: Header=BB0_5 Depth=2
	cmpq	%rax, %r13
	jne	.LBB0_18
	jmp	.LBB0_23
.LBB0_80:
	callq	abort@PLT
.Lfunc_end0:
	.size	keyboard_thread, .Lfunc_end0-keyboard_thread
	.size	.Lkeyboard_thread$local, .Lfunc_end0-keyboard_thread
                                        # -- End function
	.type	keyboard_thread.is_cap,@object  # @keyboard_thread.is_cap
	.section	.bss.keyboard_thread.is_cap,"aw",@nobits
keyboard_thread.is_cap:
	.byte	0                               # 0x0
	.size	keyboard_thread.is_cap, 1

	.type	keymap,@object                  # @keymap
	.section	.bss.keymap,"aw",@nobits
	.p2align	4, 0x0
keymap:
	.zero	102
	.size	keymap, 102

	.type	to_ascii_cap_shift,@object      # @to_ascii_cap_shift
	.section	.rodata.to_ascii_cap_shift,"a",@progbits
	.p2align	4, 0x0
to_ascii_cap_shift:
	.ascii	"\000abcdefghijklmnopqrstuvwxyz\000\000\000\000\000\000\000\000\000\000\000\000!@#$%^&*()1234567890\000\000\000\000\000\000\000\000\000\000~_+\000\t{}|\000:\"\n\000<>?\000\000\000 \000\000\000\000\000\000\000/*-+\n."
	.size	to_ascii_cap_shift, 102

	.type	to_ascii_shift,@object          # @to_ascii_shift
	.section	.rodata.to_ascii_shift,"a",@progbits
	.p2align	4, 0x0
to_ascii_shift:
	.ascii	"\000ABCDEFGHIJKLMNOPQRSTUVWXYZ\000\000\000\000\000\000\000\000\000\000\000\000!@#$%^&*()1234567890\000\000\000\000\000\000\000\000\000\000~_+\000\t{}|\000:\"\n\000<>?\000\000\000 \000\000\000\000\000\000\000/*-+\n."
	.size	to_ascii_shift, 102

	.type	to_ascii_cap,@object            # @to_ascii_cap
	.section	.rodata.to_ascii_cap,"a",@progbits
	.p2align	4, 0x0
to_ascii_cap:
	.ascii	"\000ABCDEFGHIJKLMNOPQRSTUVWXYZ\000\000\000\000\000\000\000\000\000\000\000\00012345678901234567890\000\000\000\000\000\000\000\000\000\000`-=\000\t[]\\\000;'\n\000,./\000\000\000 \000\000\000\000\000\000\000/*-+\n."
	.size	to_ascii_cap, 102

	.type	to_ascii,@object                # @to_ascii
	.section	.rodata.to_ascii,"a",@progbits
	.p2align	4, 0x0
to_ascii:
	.ascii	"\000abcdefghijklmnopqrstuvwxyz\000\000\000\000\000\000\000\000\000\000\000\00012345678901234567890\000\000\000\000\000\000\000\000\000\000`-=\000\t[]\\\000;'\n\000,./\000\000\000 \000\000\000\000\000\000\000/*-+\n."
	.size	to_ascii, 102

	.ident	"Ubuntu clang version 16.0.0 (1~exp5ubuntu3)"
	.section	".note.GNU-stack","",@progbits
