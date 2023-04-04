#include "boot.h"

/*
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memcmp.c -m32 -march=i386 -mgeneral-regs-only -S -o startup/memcmp_i386.s
gcc-me -fno-unroll-loops -fno-unroll-all-loops -fno-pie -mno-red-zone memcmp.c -m32 -march=x86-64-v3 -S
*/

#ifdef __GCC_ASM_FLAG_OUTPUTS__
# define CC_SET(c) "\n\t/* output condition code " #c "*/\n"
# define CC_OUT(c) "=@cc" #c
#else
# define CC_SET(c) "\n\tset" #c " %[_cc_" #c "]\n"
# define CC_OUT(c) [_cc_ ## c] "=qm"
#endif

int memcmp(const void *s1, const void *s2, size_t len)
{
    /*
    for (size_t i = 0; i < len; ++i) {
        if (((const uint8_t *)s1)[i] != ((const uint8_t *)s2)[i])
            return 1;
    }
    return 0;
    */
	bool diff;
	asm("repe; cmpsb" CC_SET(nz)
	    : CC_OUT(nz) (diff), "+D" (s1), "+S" (s2), "+c" (len));
	return diff;
}
