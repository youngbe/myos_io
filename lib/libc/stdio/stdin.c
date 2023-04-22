#include "stdio_impl.h"

#include <fd.h>

#undef stdin

static unsigned char buf[BUFSIZ+UNGET];
FILE __stdin_FILE = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.fd = &kernel_stdin_fd,
	.flags = F_PERM | F_NOWR,
	.read = __stdio_read,
	.seek = __stdio_seek,
	.need_lock = false,
    //.mutex : inited by code
};
FILE *const stdin = &__stdin_FILE;
FILE *volatile __stdin_used = &__stdin_FILE;
