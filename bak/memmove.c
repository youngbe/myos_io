#include "boot.h"

/*
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memmove.c -m32 -march=i386 -mgeneral-regs-only -S -o startup/memmove_i386.s
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memmove.c -m32 -march=x86-64-v3 -S
*/


void *memmove(void *dest, const void *src, size_t num)
{
    if (src > dest || (uintptr_t)dest >= (uintptr_t)src + num) {
        for (size_t i = 0; i < num; ++i)
            ((uint8_t *)dest)[i] = ((const uint8_t *)src)[i];
    }
    else if (src < dest) {
        while (num != 0) {
            --num;
            ((uint8_t *)dest)[num] = ((const uint8_t *)src)[num];
        }
    }
    return dest;
}
