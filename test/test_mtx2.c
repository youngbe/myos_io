#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

static atomic_size_t count = 0;
static mtx_t mutex;

static int thread1(void *arg)
{
    while (true) {
        for (size_t i = 0; i < 10000; ++i) {
            if (mtx_lock(&mutex) != thrd_success)
                abort();
            wait(10000);
            if (mtx_unlock(&mutex) != thrd_success)
                abort();
            wait(7 * 10000);
        }
        printf("thread1 get arg 0x%ld, count %ld:\n", (uintptr_t)arg, atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }
    return 0;
}

noreturn void kernel_main()
{
    puts("init finished, kernel started!");
    if (mtx_init(&mutex, mtx_plain) != thrd_success)
        abort();

    thrd_t threads[32];
    for (size_t i = 0; i < 32; ++i) {
        if (thrd_create(&threads[i], thread1, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }

    while (true) {

        for (size_t i = 0; i < 10000; ++i) {
            if (mtx_lock(&mutex) != thrd_success)
                abort();
            wait(10000);
            if (mtx_unlock(&mutex) != thrd_success)
                abort();
            wait(7 * 10000);
        }

        printf("printf in main, count %lu:\n", atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
