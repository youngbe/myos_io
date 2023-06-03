#include "tty-internal.h"

#include <threads.h>

static struct TTY default_tty = {
    .read_buf_ii = 0,
    .read_buf_oi = 0,
    .read_buf_used = 0,
    .read_buf_visible = 0,
    // .read_mtx
    // .read_cnd
    .write = tty_write,
    .writev = tty_writev
};

struct TTY *current_tty = &default_tty;

void kernel_init_part6()
{
    if (mtx_init(&default_tty.read_mtx, mtx_plain) != thrd_success || cnd_init(&default_tty.read_cnd) != thrd_success)
        abort();
    thrd_t keyboard_thread_t;
    if (thrd_create(&keyboard_thread_t, keyboard_thread, NULL) != thrd_success)
        abort();
}

volatile _Atomic(uint16_t) keyboard_buf[KEYBOARD_BUF_SIZE] = {0};
volatile _Atomic(uint32_t) keyboard_buf_used = 0;
volatile _Atomic(struct Thread *) keyboard_sleeping_thread = NULL;
