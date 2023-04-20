#include "stdio_impl.h"

#undef stdout
#undef stderr

#include <tty.h>
#include <stdlib.h>

static size_t __tty_write(FILE *const f, const unsigned char *const buf, const size_t size)
{
    const size_t len_f = f->wpos - f->wbase;
    if (len_f != 0)
        write(f->fd, f->wbase, len_f);
        //earlytty_write(f->wbase, len_f);
    f->wend = f->buf + f->buf_size;
    f->wpos = f->wbase = f->buf;
    write(f->fd, buf, size);
    //earlytty_write(buf, size);
    return size;
}

struct _IO_FILE __stdout_FILE = {
    .flags = F_PERM | F_NORD,
    .rpos = NULL,
    .rend = NULL,
    .wpos = NULL,
    .wend = NULL,
    .wbase = NULL,
    .write = __tty_write,
    .buf = NULL,
    .buf_size = 0,
    .mode = 0,
    .need_lock = false,
    //.mutex : inited by code
    .lbf = '\n',
    .cookie = NULL,
};
struct _IO_FILE __stderr_FILE = {
    .flags = F_PERM | F_NORD,
    .rpos = NULL,
    .rend = NULL,
    .wpos = NULL,
    .wend = NULL,
    .wbase = NULL,
    .write = __tty_write,
    .buf = NULL,
    .buf_size = 0,
    .mode = 0,
    .need_lock = false,
    //.mutex : inited by code
    .lbf = '\n',
    .cookie = NULL,
};
FILE *const stdout = (FILE *)&__stdout_FILE;
FILE *const stderr = (FILE *)&__stderr_FILE;

void kernel_init_part5(void)
{
    __stderr_FILE.need_lock = __stdout_FILE.need_lock = true;
    if (mtx_init(&__stdout_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success || mtx_init(&__stderr_FILE.mutex, mtx_plain | mtx_recursive) != thrd_success)
        abort();
}
