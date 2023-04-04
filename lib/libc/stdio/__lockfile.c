#include "stdio_impl.h"

#include <threads.h>
#include <stdlib.h>

int __lockfile(FILE *f)
{
    if (mtx_lock(&f->mutex) != thrd_success)
        abort();
    return 1;
}

void __unlockfile(FILE *f)
{
    if (mtx_unlock(&f->mutex) != thrd_success)
        abort();
}
