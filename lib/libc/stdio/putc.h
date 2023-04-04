#include "stdio_impl.h"

static inline int do_putc(int c, FILE *f)
{
    int ret;
    FLOCK(f);
    ret = putc_unlocked(c, f);
	FUNLOCK(f);
	return ret;
}
