#include "stdio_impl.h"

#include <fd.h>

#undef stdout

static unsigned char buf[BUFSIZ+UNGET];
hidden FILE __stdout_FILE = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.fd = &kernel_stdout_fd,
	.flags = F_PERM | F_NORD,
	.lbf = '\n',
	.write = __stdio_write,
	.need_lock = false,
    //.mutex : inited by code
};
FILE *const stdout = &__stdout_FILE;
FILE *volatile __stdout_used = &__stdout_FILE;

void kernel_init_part5(void)
{
    __stdin_FILE.need_lock = __stdout_FILE.need_lock = __stderr_FILE.need_lock = true;
    if (mtx_init(&__stdin_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	|| mtx_init(&__stdout_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	|| mtx_init(&__stderr_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success
	)
        abort();
}
