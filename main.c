#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

static atomic_size_t count = 0;

static int thread1(void *arg)
{
    while (true) {
        wait(1'000'000'000);
        printf("thread %lu, count %lu:\n", (uintptr_t)arg, atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }
    return 0;
}

noreturn void kernel_main()
{
    puts("init finished, kernel started!");

    thrd_t threads[32];
    for (size_t i = 0; i < 32; ++i) {
        if (thrd_create(&threads[i], thread1, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }
    for (size_t i = 0; true; ++i) {
        wait(1'000'000'000);
        printf("printf in main, count %lu:\n", atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }
    abort();

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
