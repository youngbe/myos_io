#pragma once
#include <fd.h>

#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include <stddef.h>

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

    // fd不被使用，仅是为了兼容
    ssize_t (*write)(const struct FD *fd, const void *buf, size_t size);
    ssize_t (*writev)(const struct FD *fd, const struct iovec *iov, int iovcnt);
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


// 决定键盘中断输入到哪个tty
extern struct TTY *current_tty;

static ssize_t tty_write(const struct FD *fd, const void *buf, size_t size);
static ssize_t tty_read(const struct FD *fd, void *buf, size_t size);
static ssize_t tty_readv(const struct FD *fd, const struct iovec *iov, int iovcnt);
static ssize_t tty_writev(const struct FD *fd, const struct iovec *iov, int iovcnt);
static off_t tty_lseek(const struct FD *fd, off_t offset, int whence);


// 键盘中断相关

// 这个数字需要是2的整数倍
#define KEYBOARD_BUF_SIZE 0x200000

// 存放keyboard_event_code 键盘事件代码，每个事件代表某个按键的按下或抬起
extern volatile _Atomic(uint16_t) keyboard_buf[KEYBOARD_BUF_SIZE];
extern volatile _Atomic(uint32_t) keyboard_buf_used;
extern volatile _Atomic(struct Thread *) keyboard_sleeping_thread;

int keyboard_thread(void *);

// 键盘有101个按钮
#define KEY_NUM 101

enum {
    A = 1, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _0,
    __1, __2, __3, __4, __5, __6, __7, __8, __9, __0,
    ESC, PRTSC, SCRLK, PAUSE, INSERT, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
    BACK_QUOTE, SUB, EQU, BACK,
    TAB, OPEN_BRACKET, CLOSE_BRACKET, BACKSLASH,
    CAP, SEMICOLON, QUOTE, ENTER,
    LEFT_SHIFT, COMMA, DOT, SLASH, RIGHT_SHIFT,
    LEFT_CTRL, LEFT_ALT, SPACE, RIGHT_ALT, RIGHT_CTRL,
    CURSOR_UP, CURSOR_DOWN, CURSOR_LEFT, CURSOR_RIGHT,
    __NUM, __SLASH, __ASTERISK, __SUB, __ADD, __ENTER, __DOT
};

static_assert(__DOT == KEY_NUM);
