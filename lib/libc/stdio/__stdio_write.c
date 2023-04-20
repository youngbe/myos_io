#include "stdio_impl.h"

#include <fd.h>

size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct iovec iovs[2] = {
		{ .iov_base = f->wbase, .iov_len = f->wpos-f->wbase },
		{ .iov_base = (void *)buf, .iov_len = len }
	};
	struct iovec *iov = iovs;
	size_t rem = iov[0].iov_len + iov[1].iov_len;
	int iovcnt = 2;
	ssize_t cnt;
	for (;;) {
		cnt = ({
                ssize_t ret = 0;
                for (size_t i = 0; i < iovcnt; ++i) {
                    if (iov[i].iov_len == 0)
                        continue;
                    ssize_t write_ret;
                    if (SSIZE_MAX - ret <= iov[i].iov_len) {
                        write_ret = write(f->fd, iov[i].iov_base, SSIZE_MAX - ret);
                        if (write_ret < 0)
                            ret = write_ret;
                        else
                            ret += write_ret;
                        break;
                    }
                    write_ret = write(f->fd, iov[i].iov_base, iov[i].iov_len);
                    if (write_ret < 0) {
                        ret = write_ret;
                        break;
                    }
                    else
                        ret += write_ret;
                    if (write_size != iov[i].iov_len)
                        break;
                }
                ret;
                });// syscall(SYS_writev, f->fd, iov, iovcnt);
		if (cnt == rem) {
			f->wend = f->buf + f->buf_size;
			f->wpos = f->wbase = f->buf;
			return len;
		}
		if (cnt < 0) {
			f->wpos = f->wbase = f->wend = 0;
			f->flags |= F_ERR;
			return iovcnt == 2 ? 0 : len-iov[0].iov_len;
		}
		rem -= cnt;
		if (cnt > iov[0].iov_len) {
			cnt -= iov[0].iov_len;
			iov++; iovcnt--;
		}
		iov[0].iov_base = (char *)iov[0].iov_base + cnt;
		iov[0].iov_len -= cnt;
	}
}
