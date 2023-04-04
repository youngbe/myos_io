#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

static volatile atomic_size_t tsc;

static int thread1(void *arg)
{
    __asm__ volatile("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    while (true) {
        size_t cur = rdtsc_volatile_seq();
        size_t old = atomic_exchange_explicit(&tsc, cur, memory_order_release);
        if (old > cur && old - cur > 300'000'000) {
            printf("failed in thrd %lu: old %lu cur %lu\n", (uintptr_t)arg, old, cur);
            //abort();
        }
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
    __asm__ volatile("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);

    while (true) {
        size_t cur = rdtsc_volatile_seq();
        size_t old = atomic_exchange_explicit(&tsc, cur, memory_order_release);
        if (old > cur && old - cur > 300'000'000) {
            printf("failed in main: old %lu cur %lu\n", old, cur);
            //abort();
        }
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
