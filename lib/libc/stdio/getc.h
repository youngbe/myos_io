#include "stdio_impl.h"

static inline int do_getc(FILE *f)
{
    int ret;
    FLOCK(f);
	ret = getc_unlocked(f);
	FUNLOCK(f);
}
