#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

static atomic_size_t count = 0;
static atomic_uint spin_mtx;

static int thread1(void *arg)
{
    __asm__ volatile("cli":::);
    while (true) {
        for (size_t i = 0; i < 100000; ++i) {
            {
                unsigned unlocked = 0;
                while (!atomic_compare_exchange_weak_explicit(&spin_mtx, &unlocked, 1, memory_order_acquire, memory_order_relaxed)) {
                    unlocked = 0;
                }
            }
            wait(100);
            atomic_store_explicit(&spin_mtx, 0, memory_order_release);
        }
        printf("thread1 get arg 0x%lu, count %lu:\n", (uintptr_t)arg, atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }
    return 0;
}

noreturn void kernel_main()
{
    puts("init finished, kernel started!");
    __asm__ volatile("cli":::);

    thrd_t threads[32];
    for (size_t i = 0; i < 32; ++i) {
        if (thrd_create(&threads[i], thread1, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }

    while (true) {

        for (size_t i = 0; i < 100000; ++i) {
            {
                unsigned unlocked = 0;
                while (!atomic_compare_exchange_weak_explicit(&spin_mtx, &unlocked, 1, memory_order_acquire, memory_order_relaxed)) {
                    unlocked = 0;
                }
            }
            wait(100);
            atomic_store_explicit(&spin_mtx, 0, memory_order_release);
        }

        printf("printf in main, count %lu:\n", atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
