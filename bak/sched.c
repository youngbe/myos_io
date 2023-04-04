#include "sched.h"

#include "threads.h"

#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>

extern const mi_heap_t _mi_heap_empty;
struct Thread main_thread = {1, (mi_heap_t*)&_mi_heap_empty, 0};

struct Thread* get_current_thread()
{
    size_t core_id = get_core_id();
    return threads[core_id];
}

int mtx_init(mtx_t* const mutex_in, int type)
{
    if (type != mtx_plain)
        abort();
    struct Struct_Mtx *const mutex = (struct Struct_Mtx *)mutex_in;
    mutex->spin_mutex = 0;
    mutex->is_locked = false;
    mutex->blocked_threads = NULL;
    return thrd_success;
}

static inline bool cli_tsl_lock(uint64_t *const spin_mutex)
{
    bool is_sti;
    uint64_t rflags;
    __asm__ volatile (
            "pushfq\n\t"
            "popq   %0"
            :"=g"(rflags)
            :
            :"rsp");
    is_sti = (rflags & 0x0200) == 1;
    if (is_sti) {
        __asm__ volatile (
                "cli"
                :
                :
                :"memory");
    }
    uint64_t unlocked = 0;
    const uint64_t locked = 1;
    while (!atomic_compare_exchange_weak((_Atomic uint64_t *)spin_mutex, &unlocked, locked))
        unlocked = 0;
    __asm__ volatile("":::"memory");
    return is_sti;
}

static inline void sti_tsl_unlock(uint64_t *const spin_mutex, const bool sti)
{
    __asm__ volatile("":::"memory");
    *(_Atomic uint64_t *)spin_mutex = 0;
    __asm__ volatile("":::"memory");
    if (sti) {
        __asm__ volatile (
                "sti"
                :
                :
                :"memory");
    }
}

int __attribute__ ((noinline)) mtx_lock(mtx_t* const mutex_in)
{
    struct Struct_Mtx *const mutex = (struct Struct_Mtx *)mutex_in;
    const bool is_sti = cli_tsl_lock(&mutex->spin_mutex);
    if (mutex->is_locked == false)
        mutex->is_locked = true;
    else
        abort();
    sti_tsl_unlock(&mutex->spin_mutex, is_sti);
    return thrd_success;
}

int __attribute__ ((noinline)) mtx_unlock(mtx_t* const mutex_in)
{
    struct Struct_Mtx *const mutex = (struct Struct_Mtx *)mutex_in;
    const bool is_sti = cli_tsl_lock(&mutex->spin_mutex);
    if (mutex->is_locked != false) {
        if (mutex->blocked_threads == NULL)
            mutex->is_locked = false;
        else
            abort();
    }
    sti_tsl_unlock(&mutex->spin_mutex, is_sti);
    return thrd_success;
}
