#pragma once
#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include <stddef.h>

#define TTY_READ_BUF_SIZE 0x200000
#define TTY_READ_BUF_VISIBLE_THRESHOLD (TTY_READ_BUF_SIZE >> 1)

// 1. 每个终端对应着一个 struct TTY变量 和 一个tty_thread线程 ，同时，每个struct TTY变量 和 tty_thread线程 也仅对应着一个终端
// 2. 整个操作系统有一个全局变量current_tty，决定了当前键盘中断输入的tty
// 3. tty_thread大部分时间都是阻塞的状态，当键盘输入时(由键盘中断调用tty_input_c函数)，唤醒tty_thread来处理输入字符
// 4. 每个进程有自己的stdin, stdout, stderr ，指定进程在用户态默认输出位置；内核也有全局的stdin, stdout, stderr（整个操作系统唯一），指定在内核态printf默认输出位置(相当于Linux内核printk)
// 5. FILE *指针可以对应一个TTY

// 字符流向：
// 键盘中断 -> tty -> 进程(read)
// 进程(write) -> tty
//
// 流向tty的字符将显示在屏幕上
// ungetc()函数功能由C库提供，tty不提供这种功能

// 决定键盘中断输入到哪个tty
extern struct TTY *current_tty;

struct TTY
{
    volatile atomic_char input_buf[65536];
    volatile atomic_size_t input_buf_used;
    volatile _Atomic(uint16_t) input_ii;
    // input_oi is the local var in tty_thread
    volatile _Atomic(struct Thread *)sleeping_thread;
    //struct Thread *tty_thread;

    char read_buf[TTY_READ_BUF_SIZE];
    size_t read_buf_ii;
    size_t read_buf_oi;
    size_t read_buf_used;
    size_t read_buf_visible;
    mtx_t read_mtx;
    cnd_t read_cnd;

    ssize_t (*tty_write)(const struct FD *fd, const void *, size_t);
};
