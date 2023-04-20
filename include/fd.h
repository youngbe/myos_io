#pragma once
#include <stddef.h>
#include <stdio.h>

typedef ssize_t off_t;

struct iovec {
   void* iov_base; /* Starting address */
   size_t iov_len; /* Length in bytes */
};

struct FD;

ssize_t write(const struct FD *fd, const void *buf, size_t size);
ssize_t read(const struct FD *fd, void *buf, size_t size);
ssize_t readv(const struct FD *fd, const struct iovec *iov, int iovcnt);
ssize_t writev(const struct FD *fd, const struct iovec *iov, int iovcnt);
off_t lseek(const struct FD *fd, off_t offset, int whence);
int close(struct FD *fd);
