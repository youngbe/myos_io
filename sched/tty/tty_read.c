#include "tty-internal.h"

#include <fd.h>

#include <stdlib.h>
#include <string.h>

ssize_t tty_read(const struct FD *const fd, void *const buf, size_t size)
{
    if (size == 0)
        return 0;
    struct TTY *const tty = fd->data;
    if (mtx_lock(&tty->read_mtx) != thrd_success)
        abort();
    while (tty->read_buf_visible == 0) {
        if (cnd_wait(&tty->read_cnd, &tty->read_mtx) != thrd_success)
            abort();
    }
    if (size > tty->read_buf_visible)
        size = tty->read_buf_visible;
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
    if (mtx_unlock(&tty->read_mtx) != thrd_success)
        abort();
    return size;
}
