#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef long off64_t;
typedef long off_t;

struct Thread;
struct Struct_Mtx
{
    uint64_t spin_mutex;
    bool is_locked;
    struct Thread *blocked_threads;
};
