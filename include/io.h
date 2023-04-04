#pragma once
#include <stdint.h>
#include <stddef.h>

extern unsigned __not_exist_global_sym_for_asm_seq;

static inline void outb(const uint8_t val, const uint16_t port)
{
    __asm__ volatile(
            "outb   %%al, %%dx"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"a"(val),"d"(port)
            :);
}

static inline uint8_t inb(const uint16_t port)
{
    uint8_t val;
    __asm__ volatile(
            "inb    %%dx, %%al"
            :"=a"(val), "+m"(__not_exist_global_sym_for_asm_seq)
            :"d"(port)
            :);
    return val;
}

#ifndef __WORDSIZE
#error "__WORDSIZE is not defined!"
#endif
#if __WORDSIZE == 64
static inline uint64_t rdmsr_volatile_seq(const uint32_t msr)
{
    uint64_t low;
    uint64_t high;
    __asm__ volatile(
            "rdmsr"
            :"=a"(low), "=d"(high), "+m"(__not_exist_global_sym_for_asm_seq)
            :"c"(msr)
            :);
    if (low > 0xffffffff || high > 0xffffffff)
        __builtin_unreachable();
    return (high << 32) | low;
}
static inline void wrmsr_volatile_seq(const uint32_t msr, const uint64_t val)
{
    __asm__ volatile(
            "wrmsr"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"a"(val), "d"(val >> 32), "c"(msr)
            :);
}
#elif __WORDSIZE == 32
static inline uint64_t rdmsr_volatile_seq(const uint32_t msr)
{
    uint32_t low;
    uint32_t high;
    __asm__ volatile(
            "rdmsr"
            :"=a"(low), "=d"(high), "+m"(__not_exist_global_sym_for_asm_seq)
            :"c"(msr)
            :);
    return ((uint64_t)high << 32) | low;
}
static inline void wrmsr_volatile_seq(const uint32_t msr, const uint64_t val)
{
    __asm__ volatile(
            "wrmsr"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"a"((uint32_t)val), "d"((uint32_t)(val >> 32)), "c"(msr)
            :);
}
#else
# error "unsopported wordsize!"
#endif

static inline void wait(size_t count)
{
    __asm__ volatile(
            "loop ."
            :"+c"(count), "+m"(__not_exist_global_sym_for_asm_seq)
            :
            :);
    if (count != 0)
        __builtin_unreachable();
}
