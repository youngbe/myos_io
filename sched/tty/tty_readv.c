#include "tty-internal.h"

#include <fd.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

ssize_t tty_readv(const struct FD *fd, const struct iovec *iov, int iovcnt)
{
    if (iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }
    while (true) {
        if (iovcnt == 0)
            return 0;
        if (iov->iov_len > 0)
            break;
        ++iov;
        --iovcnt;
    }
    struct TTY *const tty = fd->data;
    if (mtx_lock(&tty->read_mtx) != thrd_success)
        abort();
    while (tty->read_buf_visible == 0) {
        if (cnd_wait(&tty->read_cnd, &tty->read_mtx) != thrd_success)
            abort();
    }
    ssize_t ret = 0;

    do
    {
        size = iov->iov_len;
        buf = iov->iov_base;
        if (size > tty->read_buf_visible)
            size = tty->read_buf_visible;
        if (size > SSIZE_MAX - ret)
            size = SSIZE_MAX - ret;
        ret += size;
        tty->read_buf_visible -= size;
        tty->read_buf_used -= size;
        if (tty->read_buf_oi < TTY_READ_BUF_SIZE - size) {
            memcpy(buf, &tty->read_buf[tty->read_buf_oi], size);
            tty->read_buf_oi += size;
        } else if (tty->read_buf_oi == TTY_READ_BUF_SIZE - size) {
            memcpy(buf, &tty->read_buf[tty->read_buf_oi], size);
            tty->read_buf_oi = 0;
        } else {
            const size_t temp_size = TTY_READ_BUF_SIZE - tty->read_buf_oi;
            memcpy(buf, &tty->read_buf[tty->read_buf_oi], temp_size);
            memcpy((uint8_t *)buf + temp_size, tty->read_buf, tty->read_buf_oi = size - temp_size);
        }
        if (tty->read_buf_visible == 0 || ret == SSIZE_MAX)
            break;
        ++iov;
    } while (--iovcnt > 0);

    if (mtx_unlock(&tty->read_mtx) != thrd_success)
        abort();
    return ret;
}
