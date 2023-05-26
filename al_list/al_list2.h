// atomic_list : 原子单向链表
#pragma once

#include <misc.h>

#include <stdatomic.h>
#include <stddef.h>


// API 列表：
// al_head：获取第一个元素
// al_append: 在末尾添加
// al_appends：在末尾添加一个链表
// al_append_empty：尝试从空链表添加
// al_delete_front：获取并删除第一个元素
// al_clear：获取并删除整个链表
// al_delete_front2：获取并删除第一个元素
// al_clear2：获取并删除整个链表


// atomic list node
typedef _Atomic(void *) al_node_t;

// atomic list index
typedef struct Atomic_List_Index 
{
    _Atomic(al_node_t *) head;
    _Atomic(al_node_t *) end;
} al_index_t;



// al_head:获取当前链表第一个元素
static inline al_node_t *al_head(const al_index_t *index);



// 往链表末尾添加
// 返回值：0：原链表为空
// 1：原链表不为空
__attribute__((noinline)) int
al_append(al_index_t *list, al_node_t *element, bool is_sti);

static inline __attribute__((always_inline)) int
al_append_inline(al_t *list, al_el_t *element, bool is_sti);

__attribute__((noinline)) int
al_appends(al_t *list, al_el_t *head, al_el_t *end, bool is_sti);

static inline __attribute__((always_inline)) int
al_append_empty_inline(al_index_t *index, al_node_t *node, bool is_sti);
__attribute__((noinline)) int
al_append_empty(al_index_t *index, al_node_t *node, bool is_sti);



// al_delete_front: 获取并删除链表第一个元素
// 如果返回NULL，说明链表为空
static inline al_el_t *al_delete_front(al_t *list);
static inline al_el_t *al_delete_front2(al_t *list);



// al_clear: 清空链表并返回原链表
struct RET_al_clear
{
    al_el_t *head;
    al_el_t *end;
};
// 如果返回的end值为NULL，说明原链表为空，此时head值未定义
static inline struct RET_al_clear al_clear(al_t *list);
static inline struct RET_al_clear al_clear2(al_t *list);






inline _Atomic(void *) *al_head(const struct Atomic_List_Index *const index)
{
    return atomic_load_explicit(&index->head);
}


inline __attribute__((always_inline)) int
al_append_inline(struct Atomic_List_Index *const index, _Atomic(void *)*const node, const bool is_sti)
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
    }
    // memory_order_release: let *node write visible
    _Atomic(void *)*const old_end = atomic_exchange_explicit(&index->end, el, memory_order_release);
    if (old_end == NULL) {
        atomic_store_explicit(&index->head, node, memory_order_relaxed);
        ret = 0;
    }
    else {
        atomic_store_explicit(old_end, (void *)node, memory_order_relaxed);
        ret = 1;
    }
    if (is_sti) {
        atomic_signal_fence(memory_order_release);
        __asm__ ("sti"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    return ret;
}


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
label2:;
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

