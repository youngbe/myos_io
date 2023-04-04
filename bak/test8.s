	.text
	.file	"load_sector.c"
	.globl	test                            # -- Begin function test
	.p2align	4, 0x90
	.type	test,@function
test:                                   # @test
# %bb.0:
	pushl	%ebp
	pushl	%ebx
	pushl	%edi
	pushl	%esi
	xorl	%eax, %eax
	xorl	%ebx, %ebx
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	xorl	%esi, %esi
	xorl	%edi, %edi
	#APP
	#NO_APP
	movl	$_ebootloader_align512, %eax
	cmpl	$65537, %eax                    # imm = 0x10001
	jb	.LBB0_8
# %bb.1:
	leal	_ebootloader_align512-65536, %edx
	movl	$66, %eax
	cmpl	$65025, %edx                    # imm = 0xFE01
	jb	.LBB0_2
# %bb.3:
	movl	$65536, %ecx                    # imm = 0x10000
	.p2align	4, 0x90
.LBB0_4:                                # =>This Inner Loop Header: Depth=1
	movw	$16, ldap
	movl	$127, ldap+2
	shrl	$4, %ecx
	movw	%cx, ldap+6
	movl	%eax, ldap+8
	movl	$0, ldap+12
	#APP
	movw	$ldap, %si
	movb	$66, %ah
	movb	$128, %dl
	int	$19
	jb	.Lerror
	testb	%ah, %ah
	jne	.Lerror
	callw	.Lclear
	#NO_APP
	movl	test.sec_id_buf, %eax
	movl	test.start_buf, %ecx
	addl	$127, %eax
	movl	$_ebootloader_align512, %edx
	subl	%ecx, %edx
	addl	$65024, %ecx                    # imm = 0xFE00
	addl	$-65024, %edx                   # imm = 0xFFFF0200
	cmpl	$65025, %edx                    # imm = 0xFE01
	jb	.LBB0_6
# %bb.5:                                #   in Loop: Header=BB0_4 Depth=1
	movl	%eax, test.sec_id_buf
	movl	%ecx, test.start_buf
	jmp	.LBB0_4
.LBB0_2:
	movw	$4096, %cx                      # imm = 0x1000
	jmp	.LBB0_7
.LBB0_6:
	shrl	$4, %ecx
.LBB0_7:
	movw	$16, ldap
	shrl	$9, %edx
	movw	%dx, ldap+2
	movw	$0, ldap+4
	movw	%cx, ldap+6
	movl	%eax, ldap+8
	movl	$0, ldap+12
	#APP
	movw	$ldap, %si
	movb	$66, %ah
	movb	$128, %dl
	int	$19
	jb	.Lerror
	testb	%ah, %ah
	jne	.Lerror
	callw	.Lclear
	#NO_APP
.LBB0_8:
	popl	%esi
	popl	%edi
	popl	%ebx
	popl	%ebp
	retl
.Lfunc_end0:
	.size	test, .Lfunc_end0-test
                                        # -- End function
	.type	test.start_buf,@object          # @test.start_buf
	.data
	.p2align	2
test.start_buf:
	.long	65536                           # 0x10000
	.size	test.start_buf, 4

	.type	test.sec_id_buf,@object         # @test.sec_id_buf
	.p2align	2
test.sec_id_buf:
	.long	66                              # 0x42
	.size	test.sec_id_buf, 4

	.ident	"Ubuntu clang version 15.0.2-1"
	.section	".note.GNU-stack","",@progbits
