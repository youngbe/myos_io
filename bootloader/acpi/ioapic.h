#pragma once
#include <stdint.h>
#include <stdatomic.h>

static inline void write_ioapic_register(void *const apic_base, const uint32_t offset, const uint32_t val)
{
    atomic_store_explicit((_Atomic(uint32_t) *)apic_base, offset, memory_order_release);
    atomic_store_explicit((_Atomic(uint32_t) *)apic_base + 1, val, memory_order_release);
    /*
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]"
            :[IOREGSEL]"=m"(*(_Atomic(uint32_t) *)apic_base), "+m"(__not_exist_global_sym_for_asm_seq)
            :[offset]"ri"(offset)
            :);
    __asm__ volatile (
            "movl   %[val], %[IOWIN]"
            :[IOWIN]"=m"(((_Atomic(uint32_t) *)apic_base)[4]), "+m"(__not_exist_global_sym_for_asm_seq)
            :[val]"ri"(val)
            :);
            */
}

static inline uint32_t read_ioapic_register(void*const apic_base, const uint32_t offset)
{
    atomic_store_explicit((_Atomic(uint32_t) *)apic_base, offset, memory_order_release);
    atomic_signal_fence(memory_order_acq_rel);
    return atomic_load_explicit((_Atomic(uint32_t) *)apic_base + 1, memory_order_relaxed);
    /*
    uint32_t val;
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]"
            :[IOREGSEL]"=m"(*(_Atomic(uint32_t) *)apic_base), "+m"(__not_exist_global_sym_for_asm_seq)
            :[offset]"ri"(offset)
            :);
    __asm__ volatile (
            "movl   %[IOWIN], %[val]"
            :[val]"=r"(val), "+m"(__not_exist_global_sym_for_asm_seq)
            :[IOWIN]"m"(((volatile _Atomic(uint32_t) *)apic_base)[4])
            :);
    return val;
    */
}
