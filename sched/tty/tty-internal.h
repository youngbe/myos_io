#pragma once
#include <fd.h>

#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include <stddef.h>

#define KEYBOARD_BUF_SIZE 0x200000

#define TTY_READ_BUF_SIZE 0x200000
#define TTY_READ_BUF_VISIBLE_THRESHOLD (TTY_READ_BUF_SIZE >> 1)

// 1. 整个操作系统有一个 keyboard_buf ，用来保存键盘中断的输入
// 2. 整个操作系统有一个 keyboard_thread 线程，大部分时间是阻塞态，当有键盘输入时，出来处理键盘输入
// 3. 每个进程有自己的stdin, stdout, stderr ，指定进程在用户态默认输出位置；内核也有全局的stdin, stdout, stderr（整个操作系统唯一），指定在内核态printf默认输出位置(相当于Linux内核printk)

// 1. 每个终端对应着一个 struct TTY变量
// 2. FILE *指针可以对应一个TTY，一个TTY可以对应多个FILE *指针（类似于Linux fopen("d/dev/tty?")）

// 字符流向：
// 键盘中断 -> keyboard_thread -> tty -> 进程(read)
// 进程(write) -> tty
//
// 流向tty的字符将显示在屏幕上
// ungetc()函数功能由C库提供，tty不提供这种功能

struct TTY
{
    char read_buf[TTY_READ_BUF_SIZE];
    size_t read_buf_ii;
    size_t read_buf_oi;
    size_t read_buf_used;
    size_t read_buf_visible;
    mtx_t read_mtx;
    cnd_t read_cnd;

    ssize_t (*tty_write)(const struct FD *fd, const void *, size_t);
};

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

extern volatile atomic_char keyboard_buf[KEYBOARD_BUF_SIZE];
extern volatile atomic_size_t keyboard_buf_used;
extern volatile _Atomic(struct Thread *) keyboard_sleeping_thread;

// 决定键盘中断输入到哪个tty
extern struct TTY *current_tty;

int keyboard_thread(void *);
static ssize_t tty_write(const struct FD *fd, const void *buf, size_t size);
static ssize_t tty_read(const struct FD *fd, void *buf, size_t size);
static ssize_t tty_readv(const struct FD *fd, const struct iovec *iov, int iovcnt);
static ssize_t tty_writev(const struct FD *fd, const struct iovec *iov, int iovcnt);
static off_t tty_lseek(const struct FD *fd, off_t offset, int whence);
