#include "stdio_impl.h"

#include <threads.h>
#include <stdlib.h>

static FILE *ofl_head;
static mtx_t mtx;

FILE **__ofl_lock()
{
	if (mtx_lock(&mtx) != thrd_success)
		abort();
	return &ofl_head;
}

void __ofl_unlock()
{
	if (mtx_unlock(&mtx) != thrd_success)
		abort();
}

void kernel_init_part5(void)
{
    __stdin_FILE.need_lock = __stdout_FILE.need_lock = __stderr_FILE.need_lock = true;
    if (mtx_init(&__stdin_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	|| mtx_init(&__stdout_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	|| mtx_init(&__stderr_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	)
        abort();
    if (mtx_init(&mtx, mtx_plain) != thrd_success)
        abort();
}
