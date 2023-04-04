#include "boot.h"

/*
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memcpy.c -m32 -march=i386 -mgeneral-regs-only -S -o startup/memcpy_i386.s
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memcpy.c -m32 -march=x86-64-v3 -S
*/

void *memcpy(void *dest, const void *src, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((uint8_t *)dest)[i] = ((const uint8_t *)src)[i];
    return dest;
}
