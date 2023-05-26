#include "al_list.h"

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
