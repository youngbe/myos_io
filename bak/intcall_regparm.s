    .set BOOTLOADER_STACK_SEGMENT, 0x7000

    .set DSS_64, 0
    .set CSS_64, 8
    .set CSS_32, 16
    .set CSS_16, 24
    .set DSS_32, 32
    .set DSS_16, 40

# 此函数参考 Linux源代码 https://elixir.bootlin.com/linux/latest/source/arch/x86/boot/bioscall.S 实现
# 原版的函数是在16位实模式由下C语言程序调用，修改后的函数在32位保护模式下被C语言程序调用
# 函数首先进入16位实模式，执行int指令后返回保护模式
# 假设Bios中断满足以下条件：
#    可能修改的寄存器：%eax %ebx %ecx %edx %esi %edi %ebp %ds %es %fs %gs EFLAGS寄存器
#    不会修改的寄存器：%cs %eip %ss %esp 控制寄存器(%cr0-8 XCG0 MSR(EFER XSS)) 浮点寄存器(%mmx0-7 %zmm0-32 %st) 浮点控制寄存器MXCSR GDTR LDTR IDTR 等
    .globl  intcall
    .type   intcall, @function
intcall:
    .code32
    pushl   %ecx
    # 退出保护模式
    # 先进入保护模式下的16位模式
    movl    $DSS_16, %ecx
    movl    %ecx, %ds
    movl    %ecx, %es
    movl    %ecx, %fs
    movl    %ecx, %gs
    movl    %ecx, %ss
    ljmpl   $CSS_16, $1f
    .code16
1:
    # 关闭保护模式
    movl    %cr0, %ecx
    andl    $0xfffffffe, %ecx
    movl    %ecx, %cr0
    # 刷新段寄存器
    xorl    %ecx, %ecx
    ljmpl   $0, $1f
1:
    movw    %cx, %ds
    movw    %cx, %es
    movw    %cx, %fs
    movw    %cx, %gs
    # 配置栈
    movw    $BOOTLOADER_STACK_SEGMENT, %cx
    movw    %cx, %ss
	movzwl	%sp, %esp


	/* Self-modify the INT instruction.  Ugly, but works. */
	cmpb	%al, 3f
	je	1f
	movb	%al, 3f
	jmp	1f		/* Synchronize pipeline */
1:

    /* Save State */
    pushl   %ebx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
	pushfl

	/* Copy input state to stack frame */
	subw	$44, %sp
    movw    %dx, %si
    andl    $0xf, %esi
    shrl    $4, %edx
    movw    %dx, %ds
    movw    %ss, %ax
    movw    %ax, %es
	movw	%sp, %di
	movw	$44, %cx
	rep movsb

	/* Pop full state from the stack */
	popal
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	popfl

	/* Actual INT */
	.byte	0xcd		/* INT opcode */
3:	.byte	0

	/* Push full state to the stack */
	pushfl
	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs
	pushal

	/* Copy output state from stack frame */
	cld
	movzwl	%sp, %esp
	movl	%ss:64(%esp), %edi	/* Original %ecx == 3rd argument */
	testl	%edi, %edi
	jz	4f
    movl    %edi, %eax
    shrl    $4, %eax
    movw    %ax, %es
    andl    $0xf, %edi
    movw    %ss, %bx
    movw    %bx, %ds
	movw	%sp, %si
	movw	$44, %cx
	rep movsb
4:	addw	$44, %sp

	popfl
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %ebx

    # 回到保护模式
    # 配置栈
    testw   %sp, %sp
    jnz     1f
    movl    $(BOOTLOADER_STACK_SEGMENT << 4) + 0x10000, %esp
    jmp     2f
1:
    orl     $(BOOTLOADER_STACK_SEGMENT << 4), %esp
2:
    # 开启保护模式
    movl    %cr0, %ecx
    orl     $1, %ecx
    movl    %ecx, %cr0
    # 刷新段寄存器
    movl    $DSS_32, %ecx
    ljmpl   $CSS_32, $1f
    .code32
1:
    movl    %ecx, %ss
    movl    %ecx, %ds
    movl    %ecx, %es
    movl    %ecx, %fs
    movl    %ecx, %gs
    addl    $4, %esp     # pushl %ecx
	retl
	.size	intcall, .-intcall
