// 这个文件引用自Linux源代码：https://elixir.bootlin.com/linux/latest/source/arch/x86/boot/compressed/misc.c
// 并作了一些修改

#include "io.h"
#include "tty.h"

#include <fd.h>

#include <threads.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define CHAR_ATTR 0b00001111
#define ROWS 25
#define COLS 80

#define VIDPORT 0x3d4
#define VIDMEM ((char *)0xb8000)

static size_t x, y;

static void scroll(void)
{
	memmove(VIDMEM, VIDMEM + COLS * 2, (ROWS - 1) * COLS * 2);
	for (size_t i = (ROWS - 1) * COLS * 2; i < ROWS * COLS * 2; i += 2)
		VIDMEM[i] = '\0';
}

static inline void update_cursor(void)
{
    size_t pos;
    pos = (x + COLS * y) * 2;   /* Update cursor position */
    outb(14, VIDPORT);
    outb(0xff & (pos >> 9), VIDPORT+1);
    outb(15, VIDPORT);
    outb(0xff & (pos >> 1), VIDPORT+1);
}

static inline void __putcharx(const char c)
{
    if (c == '\n') {
        x = 0;
        if (++y >= ROWS) {
            scroll();
            y--;
        }
    } else {
        VIDMEM[(x + COLS * y) * 2] = c;
        if (++x >= COLS) {
            x = 0;
            if (++y >= ROWS) {
                scroll();
                y--;
            }
        }
    }
}

ssize_t tty_write(const struct FD *const, const void *const buf, size_t size)
{
    if (size == 0)
        return 0;
    if (size > SSIZE_MAX)
        size = SSIZE_MAX;

    if (mtx_lock(&mtx) != thrd_success)
        abort();

    for (size_t i = 0; i < size; ++i)
        __putcharx(((char *)buf)[i]);
    update_cursor();

    if (mtx_unlock(&mtx) != thrd_success)
        abort();

    return size;
}

ssize_t tty_writev(const struct FD *, const struct iovec *iov, int iovcnt)
{
    if (iovcnt < 0) {
        errno = EINVAL;
        return -1;
    }
    while (true) {
        if (iovcnt == 0)
            return 0;
        if (iov->iov_len != 0)
            break;
        --iovcnt;
        ++iov;
    }
    ssize_t ret = 0;
    if (mtx_lock(&mtx) != thrd_success)
        abort();
    for (size_t i = 0; i < iovcnt; ++i) {
        size_t size;
        if (iov[i].iov_len > SSIZE_MAX - ret)
            size = SSIZE_MAX - ret;
        else
            size = iov[i].iov_len;
        ret += size;
        for (size_t i2 = 0; i2 < size; ++i2)
            __putcharx(((char *)iov[i].iov_base)[i2]);
        if (ret == SSIZE_MAX)
            break;
    }
    update_cursor();
    if (mtx_unlock(&mtx) != thrd_success)
        abort();
    return ret;
}


// This is code for init
void kernel_init_part0(void)
{
    // 获取当前光标位置
    size_t pos;
    outb(0x0F, 0x3D4);
    pos = inb(0x3D5);
    outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;

    if (pos >= COLS * ROWS)
        pos = 0;
    else
        pos <<= 1;

    // 清空光标之后的屏幕
    for (size_t i = 0; i < pos; i += 2)
        VIDMEM[i + 1] = CHAR_ATTR;
    for (size_t i = pos; i < ROWS * COLS; i += 2) {
        VIDMEM[i] = '\0';
        VIDMEM[i + 1] = CHAR_ATTR;
    }

    x = (pos >> 1) % 80;
    y = (pos >> 1) / 80;

    if (mtx_init(&mutex, mtx_plain) != thrd_success)
        abort();
}
