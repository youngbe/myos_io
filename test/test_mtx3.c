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
        if (mtx_lock(&mutex) != thrd_success)
            abort();

        printf("======= thread %lu locked ======\n", (uintptr_t)arg);
        wait(200'000'000);
        printf("thread %lu running 1\n", (uintptr_t)arg);
        wait(200'000'000);
        printf("thread %lu running 2\n", (uintptr_t)arg);
        wait(200'000'000);
        printf("======= thread %lu unlock ======\n", (uintptr_t)arg);

        if (mtx_unlock(&mutex) != thrd_success)
            abort();
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
        if (mtx_lock(&mutex) != thrd_success)
            abort();

        printf("======= main thread locked ======\n");
        wait(200'000'000);
        printf("main thread running 1\n");
        wait(200'000'000);
        printf("main thread running 2\n");
        wait(200'000'000);
        printf("======= main thread unlock ======\n");

        if (mtx_unlock(&mutex) != thrd_success)
            abort();
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
