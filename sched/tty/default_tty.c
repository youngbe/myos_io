#include "tty-internal.h"

static struct TTY default_tty = {
    .read_buf_ii = 0,
    .read_buf_oi = 0,
    .read_buf_used = 0,
    .read_buf_visible = 0,
    .tty_write = tty_write,
};

static struct TTY *current_tty = &default_tty;
