# 输入参数:
# %ebp == 指向错误标记的地址
# %esi == xcr0 可设置位
    .section .text.ap_start64, "ax", @progbits
    .globl ap_start64
    .type ap_start64, @function
    .p2align 4, 0x90
ap_start64:
    xorl    %eax, %eax
    movq    %rax, %ds
    movq    %rax, %es
    movq    %rax, %fs
    movq    %rax, %gs
    movq    %rax, %ss

    movl    $0x80000001, %eax
    cpuid
    testl   $1 << 11, %edx
    # 不支持syscall
    je      .Lerror

    lock    andl    %esi, __ap_startup_code_xcr0_usable_bits(%rip)
    movq        __ap_startup_code_all_cores_num(%rip), %rsi
    movq        __ap_startup_code_written_xcr0_usable_bits_cores_num(%rip), %rax
1:
    cmpq        %rsi, %rax
    jae         .Lerror
    leaq        1(%rax), %rcx
    lock    cmpxchgq    %rcx, __ap_startup_code_written_xcr0_usable_bits_cores_num(%rip)
    jne         1b

    # 设置 EFER 寄存器的LME, SCE位
    movl    $0xC0000080, %ecx
    rdmsr
    # IA32_EFER.SCE[0] : 开启syscall
    orl     $1, %eax
    wrmsr

    # Check local APIC is enabled (This should be enabled defaultly)
    movl    $0x1b, %ecx
    rdmsr
    testl   $1 << 11, %eax
    je  .Lerror

    movq    $65536, %rdi
    lock    xaddq   %rdi, __ap_startup_code_core_datas(%rip)
    leaq    65536(%rdi), %rsp
    # %rdi == this_core_data

    ldmxcsr     __mxcsr_status_fast_math(%rip)
    vzeroall
    fninit

    callq   ap_init0

.Lerror:
    movl    %ebp, %ebp
    movl    $1, %eax
    xchgb   %al, (%ebp)
1:
    hlt
    jmp 1b

    .size ap_start64, . - ap_start64
