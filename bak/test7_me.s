	.file	"test.c"
	.text
	.p2align 4
	.globl	test
	.type	test, @function
test:
	movl	$_ebootloader_align512, %eax
	cmpl	$65536, %eax
	jbe	.L1
	movl	$_ebootloader_align512-65536, %edx
	movl	$66, %ecx
	cmpl	$65024, %edx
	jbe	.L6
	movl	$65536, %eax
	jmp	.L5
.L4:
	movl	%ecx, sec_id_buf.0
	movl	%eax, start_buf.1
.L5:
	xorl	%ebx, %ebx
	shrl	$4, %eax
	movl	$8323088, ldap
	movw	%bx, ldap+4
	movw	%ax, ldap+6
	movl	%ecx, ldap+8
	movl	$0, ldap+12
    # %eax == ldap.segment
    # %ebx == 0
    # %ecx == sec_id
    # %edx == cha
#APP
# 44 "test.c" 1
    xorl    %edx, %edx
	movw   $ldap, %si
	movb   $0x42, %ah
	movb   $0x80, %dl
	int    $0x13
	jc     .Lerror
	testb   %ah, %ah
	jne    .Lerror
	callw  .Lclear
# 0 "" 2
#NO_APP
	movl	sec_id_buf.0, %eax
	movl	$_ebootloader_align512, %edx
	leal	127(%eax), %ecx
	movl	start_buf.1, %eax
	addl	$65024, %eax
	subl	%eax, %edx
	cmpl	$65024, %edx
	ja	.L4
	shrl	$4, %eax
	xorl	%ebx, %ebx
	jmp	.L3
.L6:
	xorl	%ebx, %ebx
	movl	$4096, %eax
.L3:
	shrl	$9, %edx
	movw	%ax, ldap+6
	movw	%dx, ldap+2
	xorl	%edx, %edx
	movw	$16, ldap
	movw	%dx, ldap+4
	movl	%ecx, ldap+8
	movl	%ebx, ldap+12
    # %eax == ldap.segment
    # %ebx == 0
    # %ecx == sec_id
    # %edx == 0
#APP
# 74 "test.c" 1
	movw   $ldap, %si
	movb   $0x42, %ah
	movb   $0x80, %dl
	int    $0x13
	jc     .Lerror
	testb   %ah, %ah
	jne    .Lerror
.L1:
	callw  .Lclear
# 0 "" 2
#NO_APP
	.size	test, .-test
	.data
	.type	sec_id_buf.0, @object
	.size	sec_id_buf.0, 4
sec_id_buf.0:
	.long	66
	.type	start_buf.1, @object
	.size	start_buf.1, 4
start_buf.1:
	.long	65536
	.ident	"GCC: (Ubuntu 12.2.0-3ubuntu1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
