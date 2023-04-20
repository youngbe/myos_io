#pragma once

#include <tty.h>

enum fd_type_t
{
    FILE_T,
    TTY,
};

struct FD
{
    enum fd_type_t type;
    union {
        struct TTY tty;
    };
};
