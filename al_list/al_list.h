// atomic_list
// append 与 delete_front/clear/append 可并发
// delete_front/clear 与 delete_front/clear 不可并发
// 如果删除操作(delete_front/clear)之间存在并发，自己加一个自选锁
#pragma once
#include <stdatomic.h>
#include <stddef.h>

// atomic list element
typedef _Atomic(void *) al_el_t;

// atomic list
typedef struct Atomic_List 
{
    _Atomic(al_el_t *) head;
    _Atomic(al_el_t *) end;
} al_t;

// 往链表中添加
__attribute__((noinline)) void
al_append(al_t *list, al_el_t *element, bool is_sti);

static inline __attribute__((always_inline)) void
al_append_inline(al_t *list, al_el_t *element, bool is_sti);

// al_delete_front: 删除链表第一个元素并返回
// 如果返回NULL，说明链表为空
static inline al_el_t *al_delete_front(al_t *list);

// al_clear: 清空链表并返回原链表
struct RET_al_clear
{
    al_el_t *head;
    al_et_t *end;
};
// 如果返回的end值为NULL，说明原链表为空，此时head值未定义
static inline struct RET_al_clear al_clear(al_t *list);







inline _Atomic(void *)* al_delete_front(struct Atomic_List *const list)
{
    _Atomic(void *)* end = atomic_load_explicit(&list->end, memory_order_relaxed);
    if (end == NULL)
        return NULL;
    _Atomic(void *)* head;
    while ((head = atomic_load_explicit(&list->head, memory_order_relaxed)) == NULL)
        asm ("pause");
    if (head == end) {
        // list 中仅有一个元素
        *(void **)list->head = NULL;  // not atomic write
        // memory_order_release: let list->head write visible
        if (!atomic_compare_exchange_strong_explicit(&list->end, &end, NULL, memory_order_release, memory_order_relaxed))
            goto label2;
    } else {
label2:
        // list 中有多个元素
        void *next;
        while ((next = atomic_load_explicit(head, memory_order_relaxed)) == NULL)
            asm ("pause");
        *(void **)list->head = next;  // not atomic write
    }
    return head;
}

inline struct RET_al_clear al_clear(struct Atomic_List *const list)
{
    struct RET_al_clear ret;
    ret.end = atomic_load_explicit(&list->end, memory_order_relaxed);
    if (ret.end == NULL)
        return ret;
    while ((ret.head = atomic_load_explicit(&list->head, memory_order_relaxed)) == NULL)
        asm ("pause");
    *(void **)list->head = NULL;
    // memory_order_release: let list->head write visible
    ret.end = atomic_exchange_explicit(&list->end, NULL, memory_order_release);
    return ret;
}

inline __attribute__((always_inline)) void
al_append_inline(struct Atomic_List *const list, _Atomic(void *)*const el, const bool is_sti)
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
