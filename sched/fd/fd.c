#include <fd.h>

struct FD
{
    ssize_t (*read)(const struct FD *fd, void *buf, size_t size);
    ssize_t (*write)(const struct FD *fd, const void *buf, size_t size);
    ssize_t (*readv)(const struct FD *fd, const struct iovec *iov, int iovcnt);
    ssize_t (*writev)(const struct FD *fd, const struct iovec *iov, int iovcnt);
    off_t (*lseek)(const struct FD *fd, off_t offset, int whence);
    int (*close)(const struct FD *fd);
    void *data;
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
