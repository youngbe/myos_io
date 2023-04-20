#include <fd.h>

#include <tty.h>

#include <stdlib.h>

ssize_t writev(struct FD *const fd, const void *const buf, const size_t size)
{
    if (fd->type == TTY)
        return ((struct TTY *)fd->f)->write(buf, size);
    else
        abort();
}

ssize_t read(struct FD *const fd, void *const buf, const size_t size)
{
    if (fd->type == TTY)
        return tty_read(fd->f, buf, size);
    else
        abort();
}
