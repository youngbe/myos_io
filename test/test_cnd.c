#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

static atomic_size_t count = 0;
static mtx_t mtx;
static cnd_t cnd;

static int thread1(void *arg)
{
    while (true) {
        for (size_t i = 0; i < 100; ++i) {
            if (mtx_lock(&mtx) != thrd_success)
                abort();
            wait(200);
            if (cnd_wait(&cnd, &mtx) != thrd_success)
                abort();
            wait(200);
            if (mtx_unlock(&mtx) != thrd_success)
                abort();
        printf("thread1 %lu, lcount: %lu, count %lu\n", (uintptr_t)arg, i, atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
        }
    }
    return 0;
}

static int thread2(void *const arg)
{
    while (true) {
        for (size_t i = 0; i < 10000000; ++i) {
            if (cnd_broadcast(&cnd) != thrd_success)
                abort();
        }
        printf("thread2 %lu, count %lu\n", (uintptr_t)arg, atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }
    return 0;
}

noreturn void kernel_main()
{
    puts("init finished, kernel started!");
    if (mtx_init(&mtx, mtx_plain) != thrd_success) {
        fputs("failed to init mtx!\n", stderr);
        abort();
    }
    if (cnd_init(&cnd) != thrd_success) {
        fputs("failed to init cnd!\n", stderr);
        abort();
    }

    thrd_t threads[32];
    for (size_t i = 0; i < 16; ++i) {
        if (thrd_create(&threads[i], thread1, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }
    for (size_t i = 16; i < 32; ++i) {
        if (thrd_create(&threads[i], thread2, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }
    while (true) {
        for (size_t i = 0; i < 10000; ++i) {
            if (mtx_lock(&mtx) != thrd_success)
                abort();
            wait(200);
            if (cnd_wait(&cnd, &mtx) != thrd_success)
                abort();
            wait(200);
            if (mtx_unlock(&mtx) != thrd_success)
                abort();
        }
        printf("main, count %lu\n", atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
