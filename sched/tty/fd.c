#include <fd.h>
#include "tty-internal.h"

struct FD kernel_stdout_fd =
{
    .write = tty_write,
    .writev = tty_writev,
    .read = NULL,
    .readv = NULL,
    .lseek = NULL,
    .close = NULL,
    .data = NULL
};

struct FD kernel_stderr_fd =
{
    .write = tty_write,
    .writev = tty_writev,
    .read = NULL,
    .readv = NULL,
    .lseek = NULL,
    .close = NULL,
    .data = NULL
};

struct FD kernel_stdin_fd =
{
    .write = NULL,
    .writev = NULL,
    .read = tty_read,
    .readv = tty_readv,
    .lseek = NULL,
    .close = NULL,
    .data = &default_tty
};

ssize_t read(const struct FD *const fd, void *const buf, const size_t size)
{
    return fd->read(fd, buf, size);
}

ssize_t write(const struct FD *fd, const void *buf, size_t size)
{
    return fd->write(fd, buf, size);
}

ssize_t readv(const struct FD *fd, const struct iovec *iov, int iovcnt)
{
    return fd->readv(fd, iov, iovcnt);
}

ssize_t writev(const struct FD *fd, const struct iovec *iov, int iovcnt)
{
    return fd->writev(fd, iov, iovcnt);
}

off_t lseek(const struct FD *fd, off_t offset, int whence)
{
    return fd->lseek(fd, offset, whence);
}

int close(struct FD *fd)
{
    const int ret = fd->close(fd);
    free(fd);
    return ret;
}
