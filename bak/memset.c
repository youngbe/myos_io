#include "boot.h"

/*
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memset.c -m32 -march=i386 -mgeneral-regs-only -S -o startup/memset_i386.s
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memset.c -m32 -march=x86-64-v3 -S -fno-tree-loop-distribute-patterns
*/

void *memset(void *s, int c, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((uint8_t *)s)[i] = (uint8_t)c;
    return s;
}
