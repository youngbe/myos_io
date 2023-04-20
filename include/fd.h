#pragma once
#include <stddef.h>
#include <stdio.h>

enum fd_type_t
{
    FILE_T,
    TTY,
};

struct FD
{
    enum fd_type_t type;
    void *f;
};

struct iovec {
   void* iov_base; /* Starting address */
   size_t iov_len; /* Length in bytes */
};

ssize_t write(struct FD fd, const void *buf, size_t size);
ssize_t read(struct FD fd, void *buf, size_t size);
ssize_t readv(struct FD fd, const struct iovec *iov, int iovcnt);
ssize_t writev(struct FD fd, const struct iovec *iov, int iovcnt);
