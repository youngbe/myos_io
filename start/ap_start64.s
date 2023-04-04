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

    /*
{
    const uintptr_t rbx = atomic_load_explicit(&__ap_start_code_core_datas_end, memory_order_relaxed);
    uintptr_t rax = atomic_load_explicit(&__ap_start_code_core_datas, memory_order_seq_cst);
    while (true) {
        if (rax == rbx)
            goto label_error;
        if (atomic_compare_exchange_strong_explicit(&__ap_start_code_core_datas, &rax, rax + 0x10000, memory_order_seq_cst, memory_order_seq_cst))
            break;
    }
}
*/
    movq    __ap_startup_code_core_datas_end(%rip), %rcx
    movq    __ap_startup_code_core_datas(%rip), %rax
    .p2align    4, 0x90
1:
    cmpq    %rcx, %rax
    # 核心数量超过预期数量
    je  .Lerror
    leaq    65536(%rax), %rsp
    lock    cmpxchgq    %rsp, __ap_startup_code_core_datas(%rip)
    jne 1b
    # %rax == this_core_data
    movq    %rax, %rdi

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
