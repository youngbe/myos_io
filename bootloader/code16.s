    .set BOOTLOADER_STACK_SEGMENT, 0x7000

    .set DSS_64, 0
    .set CSS_64, 8
    .set CSS_32, 16
    .set CSS_16, 24
    .set DSS_32, 32
    .set DSS_16, 40

    .section .data16, "aw"
    .balign 8
_gdt:
gdt_null:
gdt_ptr:
    .word _egdt - _gdt - 1
    .long _gdt
    .word 0
gdt_code64:
    .long 0
    .byte 0
    .byte 0b10011010
    .byte 0b00100000
    .byte 0
gdt_code32:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .byte 0b11001111
    .byte 0
gdt_code16:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .word 0
gdt_data32:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11001111
    .byte 0
gdt_data16:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .word 0
_egdt:


    # 启动扇区，加载至 0x7c00
    .section .boot_sector_text, "ax", @progbits
    .code16
    .globl _start
_start:
    # 刷新段寄存器，配置堆栈
    xorl    %eax, %eax
    cli
    ljmpl   $0, $1f
1:
    movw    %ax, %ss
    xorl    %esp, %esp
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    sti
    callw   .Lclear

    # 初始化textmode(待完善)，初始化page number == 0

    # 清空屏幕
    movb    $0x06, %ah
    movb    $0x0f, %bh
    movw    $0x184f, %dx
    int     $0x10
    callw   .Lclear

    # 启用光标
    movb    $0x01, %ah
    movw    $0x000f, %cx
    int     $0x10
    callw   .Lclear

    # 初始化光标位置
    movb    $0x02, %ah
    #xorb   %bh, %bh
    #xorw    %dx, %dx
    int     $0x10
    callw   .Lclear

    .section .boot_sector_data, "aw"
1:
    .ascii  "Welcome to myos!"
2:
    .section .boot_sector_text, "ax", @progbits
    movw    $1b, %bp
    movw    $(2b - 1b), %cx
    callw   puts

    # 检测实模式下可用内存，是否到达0x7ffff
    #clc
    int     $0x12
    jc      .Lerror
    cmpw    $511, %ax
    jbe     .Lerror
    callw   .Lclear

    # 重新设置栈
    #cli
    movw    $BOOTLOADER_STACK_SEGMENT, %ax
    movw    %ax, %ss
    #xorl    %esp, %esp
    #sti

    # 检查一个扇区是不是512字节
    subw    $0x1e, %sp
    movw    $0x1e, %ss:(%esp)
    movw    %ss, %ax
    movw    %ax, %ds
    movw    %sp, %si
    movb    $0x48, %ah
    movb    $0x80, %dl
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    cmpw    $0x200, %ss:24(%esp)
    jne     .Lerror
    addw    $0x1e, %sp
    callw   .Lclear

    # 打开A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    callw   .Lclear

    # 加载 bootloader 0x7e00 - 0x10000 部分程序
    .section .boot_sector_data, "aw"

    .globl  next_load_sec_id
    .p2align 2
next_load_sec_id:
    .long 0x42

    .balign 8
    .globl  ldap
    .type   ldap,@object
ldap:
    .byte   0x10
    .byte   0
.Lread_sectors_num:
    .word   0x41
    .word   0x0
    .word   0x7e0
    .quad   1
    .size   ldap, 16
    .section .boot_sector_text, "ax", @progbits
    movl    $_ebootloader_align512, %edi
    cmpl    $0x10000, %edi
    jae     1f
    shrl    $9, %edi
    subw    $0x3e, %di
    movw    %di, next_load_sec_id
    decw    %di
    movw    %di, .Lread_sectors_num
1:
    movw    $ldap, %si
    movb    $0x42, %ah
    movb    $0x80, %dl
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    callw   .Lclear
    jmp     _start_part2

    # 重置寄存器的值， %ss %esp %cs %eip 除外
.Lclear:
    movzwl  %sp, %esp
    # 保持IF FLAG不变，其它清0
    pushfl
    movl    %ss:(%esp), %eax
    andl    $1 << 9, %eax
    orb     $1 << 1, %al
    movl    %eax, %ss:(%esp)
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    popfl
    retw

    .section .boot_sector_data, "aw"
1:
    .ascii "Error in boot sector, abort!"
2:
    .section .boot_sector_text, "ax", @progbits
.Lerror:
    callw   .Lclear
    movw    $(2b - 1b), %cx
    movw    $1b, %bp
    callw   puts
    jmp     .

#输入：%es:%bp 字符串位置
#      %cx 字符串大小
    .section .boot_sector_data, "aw"
row:
    .byte 0
    .section .boot_sector_text, "ax", @progbits
puts:
    movw    $0x1300, %ax
    movw    $0x000f, %bx
    xorb    %dl, %dl
    movb    row, %dh
    incb    row
    int     $0x10
    callw   .Lclear
    movb    $0x02, %ah
    movb    row, %dh
    int     $0x10
    jmp     .Lclear

    .section .boot_sector_magic, "aw"
    .byte 0x55
    .byte 0xaa


    .section .text16, "ax", @progbits
_start_part2:
    # 加载0x10000以上部分程序
	movl	$_ebootloader_align512, %eax
	cmpl	$65537, %eax                    # imm = 0x10001
	jb	.LBB0_8
	movl	$_ebootloader_align512-65536, %ecx
	cmpl	$65025, %ecx                    # imm = 0xFE01
	jb	.LBB0_2
	movl	$65536, %eax                    # imm = 0x10000
	movl	$66, %ecx
	.p2align	4, 0x90
.LBB0_4:                                # =>This Inner Loop Header: Depth=1
	leal	127(%ecx), %edx
	movl	%edx, next_load_sec_id
	movw	$16, ldap
	movl	$127, ldap+2
	shrl	$4, %eax
	movw	%ax, ldap+6
	movl	%ecx, ldap+8
	movl	$0, ldap+12
	movw	$ldap, %si
	movb	$66, %ah
	movb	$128, %dl
	int	$19
	jb	.Lerror
	testb	%ah, %ah
	jne	.Lerror
	callw	.Lclear
	movl	test.next_load_address, %eax
	movl	$_ebootloader_align512, %ecx
	subl	%eax, %ecx
	addl	$65024, %eax                    # imm = 0xFE00
	addl	$-65024, %ecx                   # imm = 0xFFFF0200
	cmpl	$65025, %ecx                    # imm = 0xFE01
	jb	.LBB0_6
	movl	%eax, test.next_load_address
	movl	next_load_sec_id, %ecx
	jmp	.LBB0_4
.LBB0_2:
	movw	$4096, %ax                      # imm = 0x1000
	movl	$66, %edx
	jmp	.LBB0_7
.LBB0_6:
	movl	next_load_sec_id, %edx
	shrl	$4, %eax
.LBB0_7:
	shrl	$9, %ecx
	leal	(%edx,%ecx), %esi
	movl	%esi, next_load_sec_id
	movw	$16, ldap
	movw	%cx, ldap+2
	movw	$0, ldap+4
	movw	%ax, ldap+6
	movl	%edx, ldap+8
	movl	$0, ldap+12
	movw	$ldap, %si
	movb	$66, %ah
	movb	$128, %dl
	int	$19
	jb	.Lerror
	testb	%ah, %ah
	jne	.Lerror
.LBB0_8:
	callw	.Lclear

    .section .data16, "aw"
	.type	test.next_load_address,@object  # @test.next_load_address
	.p2align	2
test.next_load_address:
	.long	65536                           # 0x10000
	.size	test.next_load_address, 4
    .section .text16, "ax", @progbits


    # 检查cpuid指令是否可用
    pushfl
    movl    %ss:(%esp), %eax
    xorl    $0x00200000, %ss:(%esp)
    popfl
    pushfl
    popl    %ecx
    # old == %eax, new == %ecx
    xorl    %eax, %ecx
    # %ecx == whichever bits were changed
    testl   $0x00200000, %ecx
    # 第21位未修改，不支持cpuid
    je      .Lcpu_failed
    # restore old eflags
    pushl   %eax
    popfl

    # 检查cpuid最高可调用参数
    # 低 cpuid >= 0dh
    xorl    %eax, %eax
    cpuid
    cmpl    $0xd, %eax
    jb  .Lcpu_failed
    # 高 cpuid >= 0x80000001
    movl    $0x80000000, %eax
    cpuid
    cmpl    $0x80000001, %eax
    jb  .Lcpu_failed

    # 检查是否支持以下特性：
    # x86-64-v3
    # apic
    # x2apic
    # msr registers exist, rdmsr/wrmsr can use
    # fsgsbase
    # 1-GByte pages in LME PAE paging
    movl    $1, %eax
    cpuid
    # edx[0]:fpu
    # edx[5]:msr
    # edx[6]:pae
    # edx[8]:cx8
    # edx[9]:apic
    # edx[15]:cmov
    # edx[23]:mmx
    # edx[24]:fxsr
    # edx[25]:sse
    # edx[26]:sse2
    movl    $(1 << 0) | (1 << 5) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 15) | (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26), %esi
    andl    %esi, %edx
    cmpl    %esi, %edx
    jne     .Lcpu_failed
    # ecx[0]:sse3
    # ecx[9]:ssse3
    # ecx[12]:fma
    # ecx[13]:cx16
    # ecx[19]:sse4.1
    # ecx[20]:sse4.2
    # ecx[21]:x2apic
    # ecx[22]:movbe
    # ecx[23]:popcnt
    # ecx[26]:xsave
    # ecx[28]:avx
    # ecx[29]:f16c
    movl    $(1 << 0) | (1 << 9) | (1 << 12) | (1 << 13) | (1 << 19)  | (1 << 20) | (1 << 21) | (1 << 22) | (1 << 23) | (1 << 26) | (1 << 28) | (1 << 29), %esi
    andl    %esi, %ecx
    cmpl    %esi, %ecx
    jne     .Lcpu_failed

    movl    $0x80000001, %eax
    cpuid
    # ecx[0]: LAHF/SAHF available in 64-bit mode, note LAHF and SAHF are always available in other modes, regardless of the enumeration of this feature flag
    # ecx[5]: lzcnt
    movl    $(1 << 0) | (1 << 5), %esi
    andl    %esi, %ecx
    cmpl    %esi, %ecx
    jne      .Lcpu_failed
    # edx[26]: pdpe1gb : 1-GByte pages in LME PAE paging
    # edx[29]: lm
    movl    $(1 << 26) | (1 << 29), %esi
    andl    %esi, %edx
    cmpl    %esi, %edx
    jne     .Lcpu_failed

    movl    $7, %eax
    xorl    %ecx, %ecx
    cpuid
    # ebx[0]:FSGSBASE
    # ebx[3]:bmi1
    # ebx[5]:avx2
    # ebx[8]:bmi2
    movl    $(1 << 0) | (1 << 3) | (1 << 5) | (1 << 8), %esi
    andl    %esi, %ebx
    cmpl    %esi, %ebx
    jne     .Lcpu_failed

    # 启用OSFXSR,fsgsbase,OSXSAVE
    movl    %cr4, %eax
    # cr4[9].OSFXSR
    # cr4[16].fsgsbase
    # cr4[18].OSXSAVE
    orl     $(1 << 9) | (1 << 16) | (1 << 18), %eax
    movl    %eax, %cr4

    # 重新检查CPUID OSXSAVE
    movl    $1, %eax
    cpuid
    # ecx[27].OSXSAVE
    testl   $1 << 27, %ecx
    je      .Lcpu_failed

    # CPUID with %eax == 0xd, %ecx == 0, 返回xcr0可设置位
    movl    $0xd, %eax
    xorl    %ecx, %ecx
    cpuid
    # xcr0[0]: x87
    # xcr0[1]: sse
    # xcr0[2]: avx
    movl    $0b111, %edi
    andl    %edi, %eax
    cmpl    %edi, %eax
    jne     .Lcpu_failed
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xsetbv


    # 进入保护模式
    # 关中断
    cli
    # 加载 gdt
    lgdtl   gdt_ptr
    movl    $DSS_32, %eax
    # 开启保护模式
    # cr0[0].PE : 1 to enable protected mode
    # cr0[1].MP : 1 to enable x87 && mmx
    # CR0[2].EM: 0 to enable x87 && mmx
    # CR0[3].TS: 0 to enable x87 && mmx
    movl    %cr0, %ecx
    andl    $~0b1100, %ecx
    orl     $0b11, %ecx
    movl    %ecx, %cr0
    # 配置栈
    movl    $(BOOTLOADER_STACK_SEGMENT << 4) + 0x10000, %esp
    # long jump and refresh segment registers
    ljmpl   $CSS_32, $1f
    .code32
1:
    movl    %eax, %ss
    movl    %eax, %ds
    movl    %eax, %es
    movl    %eax, %fs
    movl    %eax, %gs

    # clear registers status
    .section .data16, "aw"
.Lmxcsr_fast_math:
    # set MXCSR[15].FZ = 1: for fast-math
    # set MXCSR[6].DAZ = 1: for fast-math
    .long (1 << 6) | (0b111111 << 7) | (1 << 15)
    .section .text16, "ax", @progbits
    ldmxcsr     .Lmxcsr_fast_math
    vzeroall
    fninit
    calll   console_init
    calll   bootloader_main

    .code16
    .section .data16, "aw"
1:
    .ascii "The cpu is not support fot this OS, abort!"
2:
    .section .text16, "ax", @progbits
.Lcpu_failed:
    callw   .Lclear
    movw    $(2b - 1b), %cx
    movw    $1b, %bp
    callw   puts
    jmp     .


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


    movl    %ss:4(%esp), %eax
    movl    %ss:8(%esp), %edx
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
	movl	%ss:76(%esp), %edi	/* Original %ecx == 3rd argument */
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
	retl
	.size	intcall, .-intcall
