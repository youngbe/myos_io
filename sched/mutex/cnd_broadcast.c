#include "threads.h"
#include "sched-internal.h"

static struct RET_al_clear
cnd_broadcast_part0(void *, void *, al_index_t *const threads)
{
    return al_clear(threads);
}

static inline void __list_add(al_node_t **const phead, al_node_t **const pend, size_t *const count, struct Thread *const thread)
{
    if (*pend == NULL)
        *phead = *pend = &thread->al_node;
    else {
        *(void **)*pend = &thread->al_node;
        *pend = &thread->al_node;
    }
    ++*count;
}

__attribute__((noinline))
int cnd_broadcast(struct Cond *const cond)
{
    const bool is_sti = check_sti();
    struct RET_al_clear ret;
    if (is_sti)
        ret = ((__typeof__(&cnd_broadcast_part0))cli_spinlock_do)(cnd_broadcast_part0, &cond->spin_mutex, &cond->threads);
    else
        ret = ((__typeof__(&cnd_broadcast_part0))spinlock_do)(cnd_broadcast_part0, &cond->spin_mutex, &cond->threads);
    if (ret.end == NULL) {
        // 没有线程
        return thrd_success;
    }
    void *node = ret.head;
    al_node_t *new_schedulable_threads_head;
    al_node_t *new_schedulable_threads_end = NULL;
    size_t count = 0;
    void *next_node;
    do {
        if (node == ret.end)
            next_node = NULL;
        else {
            while ((next_node = atomic_load_explicit((_Atomic(void *) *)node, memory_order_relaxed)) == NULL)
                __asm__ volatile ("pause");
        }
        struct Thread *const thread = list_entry(node, struct Thread, al_node);
        struct Mutex *const mutex = thread->temp0;
        if (mutex == NULL) {
            __list_add(&new_schedulable_threads_head, &new_schedulable_threads_end, &count, thread);
            continue;
        }
        // 上锁
        void *const mutex_node = &thread->temp0;
        void *current_wait_end = atomic_load_explicit(&mutex->wait_end, memory_order_relaxed);
        if (current_wait_end == NULL) {
            if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &current_wait_end, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed)) {
label1:
                atomic_store_explicit(&mutex->owner, thread, memory_order_relaxed);
                __list_add(&new_schedulable_threads_head, &new_schedulable_threads_end, &count, thread);
                continue;
            }
        }
        *(void **)mutex_node = NULL;
        void *const last_end = atomic_exchange_explicit(&mutex->wait_end, mutex_node, memory_order_release);
        if (last_end == NULL) {
            // 成为了owner
            void *temp = mutex_node;
            if (atomic_compare_exchange_strong_explicit(&mutex->wait_end, &temp, (void *)&mutex->waiters, memory_order_relaxed, memory_order_relaxed))
                goto label1;
            atomic_store_explicit(&mutex->owner, thread, memory_order_relaxed);
            __list_add(&new_schedulable_threads_head, &new_schedulable_threads_end, &count, thread);
            while ((temp = atomic_load_explicit((_Atomic(void *) *)mutex_node, memory_order_relaxed)) == NULL)
                asm("pause");
            *(void **)&mutex->waiters = temp;
        } else {
            *(void **)&thread->al_node = NULL;
            atomic_store_explicit((_Atomic(void *) *)last_end, mutex_node, memory_order_release);
        }
    } while ((node = next_node) != NULL);

    if (new_schedulable_threads_end != NULL) {
        *(void **)new_schedulable_threads_end = NULL;
        if (is_sti)
            cli_set_threads_schedulable(new_schedulable_threads_head, new_schedulable_threads_end, count);
        else
            set_threads_schedulable(new_schedulable_threads_head, new_schedulable_threads_end, count);
    }
    return thrd_success;
}
