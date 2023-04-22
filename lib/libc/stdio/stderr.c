#include "stdio_impl.h"

#include <fd.h>

#undef stderr

static unsigned char buf[UNGET];
FILE __stderr_FILE = {
	.buf = buf+UNGET,
	.buf_size = 0,
	.fd = &kernel_stderr_fd,
	.flags = F_PERM | F_NORD,
	.lbf = -1,
	.write = __stdio_write,
	.seek = __stdio_seek,
	.need_lock = false,
    //.mutex : inited by code
};
FILE *const stderr = &__stderr_FILE;
FILE *volatile __stderr_used = &__stderr_FILE;
