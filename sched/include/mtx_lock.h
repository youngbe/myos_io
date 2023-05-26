#pragma once

#include <threads.h>
#include "sched-internal.h"

#include <stdatomic.h>

static inline int mtx_trylockx(struct Mutex *const mutex, const struct Thread *const current_thread, const bool is_sti)
{
    const struct Thread *const current_owner = al_head(&mutex->threads);
    if (mtx_owner == current_thrd) {
        if (mtx->count == 0 || mtx->count == SIZE_MAX)
            return thrd_error;
        else {
            ++mtx->count;
            atomic_signal_fence(memory_order_acquire);
            return thrd_success;
        }
    }
    else if (current_owner == NULL) {
        if (al_append_empty(&mutex->threads, &current_thread->default_al_node) == 0)
            return thrd_success;
    }
    return thrd_busy;
}
