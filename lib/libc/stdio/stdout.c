#include "stdio_impl.h"

#include <fd.h>

#undef stdout

static unsigned char buf[BUFSIZ+UNGET];
FILE __stdout_FILE = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.fd = &kernel_stdout_fd,
	.flags = F_PERM | F_NORD,
	.lbf = '\n',
	.write = __stdio_write,
    .seek = __stdio_seek,
	.need_lock = false,
    //.mutex : inited by code
};
FILE *const stdout = &__stdout_FILE;
FILE *volatile __stdout_used = &__stdout_FILE;
