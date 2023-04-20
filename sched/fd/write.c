#include <fd.h>
#include "fd-internal.h"

#include <stdlib.h>

size_t write(struct FD *const fd, const void *const buf, const size_t size)
{
    if (fd->type == TTY)
        return fd->tty.write(buf, size);
    else
        abort();
}

size_t read(struct FD *const fd, void *const buf, const size_t size)
{
    if (fd->type == TTY)
        return tty_read(&fd->tty, buf, size);
    else
        abort();
}
