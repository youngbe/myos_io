#pragma once
#include <threads.h>

static inline thrd_t thrd_currentx(void)
{
    thrd_t current_thread;
    __asm__ volatile(
            "movq   %%gs:0, %0"
            :"=r"(current_thread)
            :
            :);
    return current_thread;
}
