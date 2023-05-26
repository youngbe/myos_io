#include "al_list.h"

__attribute__((noinline)) int
al_append_empty(struct Atomic_List_Index *const index, _Atomic(void *) *const node, const bool is_sti)
{
    int ret;
    // *node = NULL , not atomic write
    *(void **)node = NULL;
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ ("cli"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
        atomic_signal_fence(memory_order_acquire);
    }
    _Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    // memory_order_release: let *node = NULL visible
    if (end == NULL && atomic_compare_exchange_strong_explicit(&index->end, &end, node, memory_order_release, memory_order_relaxed)) {
        atomic_store_explicit(&index->head, node, memory_order_relaxed);
        ret = 0;
    } else
        ret = 1;
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ ("sti"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    return ret;
}

__attribute__((no_inline)) void
al_append(struct Atomic_List *const list, _Atomic(void *)*const el, const bool is_sti)
{
    // *el = NULL , not atomic write
    *(void **)el = NULL;
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ ("cli"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    // memory_order_release: let *el write visible
    _Atomic(void *)*const old_end = atomic_exchange_explicit(&list->end, el, memory_order_release);
    if (old_end == NULL)
        atomic_store_explicit(&list->head, el, memory_order_relaxed);
    else
        atomic_store_explicit(old_end, (void *)el, memory_order_relaxed);
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ ("sti"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
}
