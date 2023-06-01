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
_Atomic(void *) current_owner = NULL;

__attribute__((noinline))
void __lock_failed()
{
    __asm__ volatile ("");
    abort();
}

__attribute__((noinline))
void __lock_error()
{
    __asm__ volatile ("");
    abort();
}

static int thread1(void *arg)
{
    const struct Thread *const thread = thrd_current();
    void *temp;
    while (true) {
        for (size_t i = 0; i < 1000; ++i) {
            if (mtx_lock(&mutex) != thrd_success) {
                __asm__ volatile ("");
                __lock_error();
                abort();
            }
            temp = NULL;
            if (!atomic_compare_exchange_strong_explicit(&current_owner, &temp, thread, memory_order_relaxed, memory_order_relaxed)) {
                __asm__ volatile ("");
                __lock_failed();
                abort();
            }
            wait(500);
            temp = thread;
            if (!atomic_compare_exchange_strong_explicit(&current_owner, &temp, NULL, memory_order_relaxed, memory_order_relaxed)) {
                __asm__ volatile ("");
                __lock_failed();
                abort();
            }
            if (mtx_unlock(&mutex) != thrd_success) {
                __asm__ volatile ("");
                __lock_error();
                abort();
            }
            wait(7 * 500);
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
    if (mtx_lock(&mutex) != thrd_success)
        abort();

    thrd_t threads[32];
    for (size_t i = 0; i < 32; ++i) {
        if (thrd_create(&threads[i], thread1, (void *)i) != thrd_success) {
            fputs("failed to create thread!\n", stderr);
            abort();
        }
    }
    if (mtx_unlock(&mutex) != thrd_success)
        abort();

    const struct Thread *const thread = thrd_current();
    void *temp;

    while (true) {

        for (size_t i = 0; i < 1000; ++i) {
            if (mtx_lock(&mutex) != thrd_success) {
                __asm__ volatile ("");
                __lock_error();
                abort();
            }
            temp = NULL;
            if (!atomic_compare_exchange_strong_explicit(&current_owner, &temp, thread, memory_order_relaxed, memory_order_relaxed)) {
                __asm__ volatile ("");
                __lock_failed();
                abort();
            }
            wait(500);
            temp = thread;
            if (!atomic_compare_exchange_strong_explicit(&current_owner, &temp, NULL, memory_order_relaxed, memory_order_relaxed)) {
                __asm__ volatile ("");
                __lock_failed();
                abort();
            }
            if (mtx_unlock(&mutex) != thrd_success) {
                __asm__ volatile ("");
                __lock_error();
                abort();
            }
            wait(7 * 500);
        }
        printf("printf in main, count %lu:\n", atomic_fetch_add_explicit(&count, 1, memory_order_relaxed));

    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
