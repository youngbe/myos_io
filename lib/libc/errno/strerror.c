#include <errno.h>

/* mips has one error code outside of the 8-bit range due to a
 * historical typo, so we just remap it. */
#if EDQUOT==1133
#define EDQUOT_ORIG 1133
#undef  EDQUOT
#define EDQUOT 109
#endif

static const char *const_strerr_array[] = {
#define E(n, s) [n] = s,
#include "__strerror.h"
#undef E
};

char *strerror(int e)
{
#ifdef EDQUOT_ORIG
    if (e==EDQUOT) e=0;
    else if (e==EDQUOT_ORIG) e=EDQUOT;
#endif
    if (e >= sizeof const_strerr_array / sizeof *const_strerr_array) e = 0;
    return (char *)const_strerr_array[e];
}
