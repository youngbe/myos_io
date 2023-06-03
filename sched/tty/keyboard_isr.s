	.section	.text.keyboard_isr,"ax",@progbits
	.globl	keyboard_isr                    # -- Begin function keyboard_isr
	.p2align	4, 0x90
	.type	keyboard_isr,@function
keyboard_isr:                           # @keyboard_isr
	pushq	%rax
	pushq	%r8
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	leaq	48(%rsp), %rdx
	movl	keyboard_buf_used(%rip), %eax
	.p2align	4, 0x90
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	cmpl	$2097152, %eax                  # imm = 0x200000
	je	.LBB0_32
# %bb.2:                                #   in Loop: Header=BB0_1 Depth=1
	movl	%eax, %ecx
	leal	1(%rcx), %esi
	lock		cmpxchgl	%esi, keyboard_buf_used(%rip)
	jne	.LBB0_1
# %bb.3:
	xorl	%edi, %edi
	testl	%ecx, %ecx
	je	.LBB0_5
# %bb.4:
	xorl	%ecx, %ecx
	movw	$96, %dx
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-31, %al
	jne	.LBB0_13
	jmp	.LBB0_24
.LBB0_5:
	movl	$1, %r8d
	xorl	%eax, %eax
	lock		cmpxchgq	%r8, keyboard_sleeping_thread(%rip)
	movl	$0, %ecx
	je	.LBB0_12
# %bb.6:
	movq	%rax, %rsi
	testb	$3, 8(%rdx)
	jne	.LBB0_8
# %bb.7:
	#APP
	movq	%gs:0, %r8
	#NO_APP
	testq	%r8, %r8
	je	.LBB0_11
.LBB0_8:
	movq	keyboard_sleeping_thread(%rip), %rax
	movq	%rax, %rcx
	leaq	schedulable_threads(%rip), %rdx
	xchgq	%rcx, schedulable_threads+8(%rip)
	testq	%rcx, %rcx
	cmovneq	%rcx, %rdx
	movq	%rax, (%rdx)
	lock		incq	schedulable_threads_num(%rip)
    lfence
	cmpq	$0, schedulable_threads_num(%rip)
	je	.LBB0_10
	cmpq	$0, idle_cores_num(%rip)
	je	.LBB0_10
# %bb.9:
	movl	$802851, %eax                   # imm = 0xC4023
	xorl	%edx, %edx
	movl	$2096, %ecx                     # imm = 0x830
	#APP
	wrmsr
	#NO_APP
.LBB0_10:
	lock		incq	old_schedulable_threads_num(%rip)
	movq	%rsi, %rdi
	movq	%r8, %rcx
.LBB0_12:
	movw	$96, %dx
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-31, %al
	je	.LBB0_24
.LBB0_13:
	movzbl	%al, %edx
	cmpl	$224, %edx
	jne	.LBB0_14
# %bb.16:
	movw	$96, %dx
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-73, %al
	je	.LBB0_21
# %bb.17:
	movzbl	%al, %edx
	cmpl	$42, %edx
	jne	.LBB0_18
# %bb.19:
	movw	$96, %dx
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-32, %al
	jne	.LBB0_23
# %bb.20:
	#APP
	inb	%dx, %al
	#NO_APP
	movw	$121, %dx
	cmpb	$55, %al
	je	.LBB0_29
	jmp	.LBB0_23
.LBB0_14:
	leaq	ps2_set1_map(%rip), %rdx
	jmp	.LBB0_15
.LBB0_21:
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-32, %al
	jne	.LBB0_23
# %bb.22:
	#APP
	inb	%dx, %al
	#NO_APP
	movw	$120, %dx
	cmpb	$-86, %al
	je	.LBB0_29
	jmp	.LBB0_23
.LBB0_18:
	leaq	ps2_set1_map2(%rip), %rdx
.LBB0_15:
	movzbl	%al, %eax
	movzwl	(%rdx,%rax,2), %edx
	testw	%dx, %dx
	jne	.LBB0_29
	jmp	.LBB0_23
.LBB0_11:
	lock		decq	idle_cores_num(%rip)
	xorl	%ecx, %ecx
	movq	%rsi, %rdi
	movw	$96, %dx
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-31, %al
	jne	.LBB0_13
.LBB0_24:
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$29, %al
	jne	.LBB0_23
# %bb.25:
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$69, %al
	jne	.LBB0_23
# %bb.26:
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-31, %al
	jne	.LBB0_23
# %bb.27:
	#APP
	inb	%dx, %al
	#NO_APP
	cmpb	$-99, %al
	jne	.LBB0_23
# %bb.28:
	#APP
	inb	%dx, %al
	#NO_APP
	movw	$125, %dx
	cmpb	$-59, %al
	jne	.LBB0_23
.LBB0_29:
	movl	$1, %eax
	lock		xaddl	%eax, keyboard_buf_ii(%rip)
	movzwl	%ax, %eax
	leaq	keyboard_buf(%rip), %rsi
	movw	%dx, (%rsi,%rax,2)
	testq	%rdi, %rdi
	je	.LBB0_32
# %bb.30:
	testq	%rcx, %rcx
	je	.LBB0_31
.LBB0_32:
	xorl	%eax, %eax
	xorl	%edx, %edx
	movl	$2059, %ecx                     # imm = 0x80B
	#APP
	wrmsr
	#NO_APP
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%r8
	popq	%rax
	iretq
.LBB0_23:
	#APP
	callq	abort
	#NO_APP
.LBB0_31:
	#APP
	movq	%gs:8, %rsi
	jmp	switch_to_interrupt
	#NO_APP
.Lfunc_end0:
	.size	keyboard_isr, .Lfunc_end0-keyboard_isr
                                        # -- End function
	.type	keyboard_buf_ii,@object         # @keyboard_buf_ii
	.section	.bss.keyboard_buf_ii,"aw",@nobits
	.p2align	2, 0x0
keyboard_buf_ii:
	.long	0                               # 0x0
	.size	keyboard_buf_ii, 4

	.type	ps2_set1_map2,@object           # @ps2_set1_map2
	.section	.rodata.ps2_set1_map2,"a",@progbits
	.p2align	4, 0x0
ps2_set1_map2:
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	201                             # 0xc9
	.short	181                             # 0xb5
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	193                             # 0xc1
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	179                             # 0xb3
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	129                             # 0x81
	.short	183                             # 0xb7
	.short	131                             # 0x83
	.short	0                               # 0x0
	.short	187                             # 0xbb
	.short	0                               # 0x0
	.short	189                             # 0xbd
	.short	0                               # 0x0
	.short	135                             # 0x87
	.short	185                             # 0xb9
	.short	137                             # 0x89
	.short	127                             # 0x7f
	.short	133                             # 0x85
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	200                             # 0xc8
	.short	180                             # 0xb4
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	192                             # 0xc0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	178                             # 0xb2
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	128                             # 0x80
	.short	182                             # 0xb6
	.short	130                             # 0x82
	.short	0                               # 0x0
	.short	186                             # 0xba
	.short	0                               # 0x0
	.short	188                             # 0xbc
	.short	0                               # 0x0
	.short	134                             # 0x86
	.short	184                             # 0xb8
	.short	136                             # 0x88
	.short	126                             # 0x7e
	.short	132                             # 0x84
	.zero	86
	.size	ps2_set1_map2, 510

	.type	ps2_set1_map,@object            # @ps2_set1_map
	.section	.rodata.ps2_set1_map,"a",@progbits
	.p2align	4, 0x0
ps2_set1_map:
	.short	0                               # 0x0
	.short	119                             # 0x77
	.short	79                              # 0x4f
	.short	81                              # 0x51
	.short	83                              # 0x53
	.short	85                              # 0x55
	.short	87                              # 0x57
	.short	89                              # 0x59
	.short	91                              # 0x5b
	.short	93                              # 0x5d
	.short	95                              # 0x5f
	.short	97                              # 0x61
	.short	141                             # 0x8d
	.short	143                             # 0x8f
	.short	145                             # 0x91
	.short	147                             # 0x93
	.short	35                              # 0x23
	.short	47                              # 0x2f
	.short	11                              # 0xb
	.short	37                              # 0x25
	.short	41                              # 0x29
	.short	51                              # 0x33
	.short	43                              # 0x2b
	.short	19                              # 0x13
	.short	31                              # 0x1f
	.short	33                              # 0x21
	.short	149                             # 0x95
	.short	151                             # 0x97
	.short	161                             # 0xa1
	.short	173                             # 0xad
	.short	3                               # 0x3
	.short	39                              # 0x27
	.short	9                               # 0x9
	.short	13                              # 0xd
	.short	15                              # 0xf
	.short	17                              # 0x11
	.short	21                              # 0x15
	.short	23                              # 0x17
	.short	25                              # 0x19
	.short	157                             # 0x9d
	.short	159                             # 0x9f
	.short	139                             # 0x8b
	.short	163                             # 0xa3
	.short	153                             # 0x99
	.short	53                              # 0x35
	.short	49                              # 0x31
	.short	7                               # 0x7
	.short	45                              # 0x2d
	.short	5                               # 0x5
	.short	29                              # 0x1d
	.short	27                              # 0x1b
	.short	165                             # 0xa5
	.short	167                             # 0xa7
	.short	169                             # 0xa9
	.short	171                             # 0xab
	.short	195                             # 0xc3
	.short	175                             # 0xaf
	.short	177                             # 0xb1
	.short	155                             # 0x9b
	.short	55                              # 0x37
	.short	57                              # 0x39
	.short	59                              # 0x3b
	.short	61                              # 0x3d
	.short	63                              # 0x3f
	.short	65                              # 0x41
	.short	67                              # 0x43
	.short	69                              # 0x45
	.short	71                              # 0x47
	.short	73                              # 0x49
	.short	191                             # 0xbf
	.short	123                             # 0x7b
	.short	111                             # 0x6f
	.short	113                             # 0x71
	.short	115                             # 0x73
	.short	197                             # 0xc5
	.short	105                             # 0x69
	.short	107                             # 0x6b
	.short	109                             # 0x6d
	.short	199                             # 0xc7
	.short	99                              # 0x63
	.short	101                             # 0x65
	.short	103                             # 0x67
	.short	117                             # 0x75
	.short	203                             # 0xcb
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	75                              # 0x4b
	.short	77                              # 0x4d
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	118                             # 0x76
	.short	78                              # 0x4e
	.short	80                              # 0x50
	.short	82                              # 0x52
	.short	84                              # 0x54
	.short	86                              # 0x56
	.short	88                              # 0x58
	.short	90                              # 0x5a
	.short	92                              # 0x5c
	.short	94                              # 0x5e
	.short	96                              # 0x60
	.short	140                             # 0x8c
	.short	142                             # 0x8e
	.short	144                             # 0x90
	.short	146                             # 0x92
	.short	34                              # 0x22
	.short	46                              # 0x2e
	.short	10                              # 0xa
	.short	36                              # 0x24
	.short	40                              # 0x28
	.short	50                              # 0x32
	.short	42                              # 0x2a
	.short	18                              # 0x12
	.short	30                              # 0x1e
	.short	32                              # 0x20
	.short	148                             # 0x94
	.short	150                             # 0x96
	.short	160                             # 0xa0
	.short	172                             # 0xac
	.short	2                               # 0x2
	.short	38                              # 0x26
	.short	8                               # 0x8
	.short	12                              # 0xc
	.short	14                              # 0xe
	.short	16                              # 0x10
	.short	20                              # 0x14
	.short	22                              # 0x16
	.short	24                              # 0x18
	.short	156                             # 0x9c
	.short	158                             # 0x9e
	.short	138                             # 0x8a
	.short	162                             # 0xa2
	.short	152                             # 0x98
	.short	52                              # 0x34
	.short	48                              # 0x30
	.short	6                               # 0x6
	.short	44                              # 0x2c
	.short	4                               # 0x4
	.short	28                              # 0x1c
	.short	26                              # 0x1a
	.short	164                             # 0xa4
	.short	166                             # 0xa6
	.short	168                             # 0xa8
	.short	170                             # 0xaa
	.short	194                             # 0xc2
	.short	174                             # 0xae
	.short	176                             # 0xb0
	.short	154                             # 0x9a
	.short	54                              # 0x36
	.short	56                              # 0x38
	.short	58                              # 0x3a
	.short	60                              # 0x3c
	.short	62                              # 0x3e
	.short	64                              # 0x40
	.short	66                              # 0x42
	.short	68                              # 0x44
	.short	70                              # 0x46
	.short	72                              # 0x48
	.short	190                             # 0xbe
	.short	122                             # 0x7a
	.short	110                             # 0x6e
	.short	112                             # 0x70
	.short	114                             # 0x72
	.short	196                             # 0xc4
	.short	104                             # 0x68
	.short	106                             # 0x6a
	.short	108                             # 0x6c
	.short	198                             # 0xc6
	.short	98                              # 0x62
	.short	100                             # 0x64
	.short	102                             # 0x66
	.short	116                             # 0x74
	.short	202                             # 0xca
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	0                               # 0x0
	.short	74                              # 0x4a
	.short	76                              # 0x4c
	.zero	76
	.size	ps2_set1_map, 510