// atomic_list : 原子单向链表
#pragma once

#include <misc.h>

#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>


// API 列表：
// al_index_init：初始化index
// al_node_init：初始化node
// al_head：获取第一个元素
// al_append: 在末尾添加，调用时中断处于关闭状态
// cli_al_append: 在末尾添加，调用时中断处于开启状状态
// al_append_empty：尝试从空链表添加
// cli_al_append_empty：尝试从空链表添加
// al_delete_front：获取并删除第一个元素
// al_clear：获取并删除整个链表
// al_delete_front2：获取并删除第一个元素
// al_clear2：获取并删除整个链表
// al_appends：在末尾添加一个链表
// al_appends_sti：在末尾添加一个链表


// atomic list node
typedef _Atomic(void *) al_node_t;

// atomic list index
typedef struct Atomic_List_Index 
{
    _Atomic(al_node_t *) head;
    _Atomic(al_node_t *) end;
} al_index_t;

#define AL_INDEX_INIT_VAL {NULL, NULL}
#define AL_NODE_INIT_VAL NULL


// al_index_init: 初始化index
static inline void al_index_init(al_index_t *index);


// al_head:获取当前链表第一个元素
static inline al_node_t *al_head(const al_index_t *index);



// 往链表末尾添加
// 返回值：0：原链表为空
// 1：原链表不为空
// 如果原链表为空，则memory_order_acq_rel
// 否则memory_order_release
__attribute__((noinline)) int
al_append(al_index_t *index, al_node_t *node);

__attribute__((noinline)) int
al_append_empty(al_index_t *index, al_node_t *node);

__attribute__((noinline)) int
al_appends(al_index_t *index, al_node_t *head, al_node_t *end);



// al_delete_front: 获取并删除链表第一个元素
// 如果返回NULL，说明链表为空
// 如果成功删除最后一个，则memory_order_acq_rel
// 如果成功删除，则 memory_order_acquire
struct RET_al_delete_front
{
    al_node_t *head;
    al_node_t *next;
};
static inline struct RET_al_delete_front al_delete_front(al_index_t *index);
static inline struct RET_al_delete_front al_delete_front2(al_index_t *index);



// al_clear: 清空链表并返回原链表
// memory_order_acq_rel
struct RET_al_clear
{
    al_node_t *head;
    al_node_t *end;
};
// 如果返回的end值为NULL，说明原链表为空，此时head值未定义
static inline struct RET_al_clear al_clear(al_index_t *index);
static inline struct RET_al_clear al_clear2(al_index_t *index);





static inline void al_index_init(struct Atomic_List_Index *const index)
{
    *(void **)&index->head = NULL;
    *(void **)&index->end = NULL;
}

static inline void al_node_init(_Atomic(void *) *const node)
{
    *(void **)node = NULL;
}

inline _Atomic(void *) *al_head(const struct Atomic_List_Index *const index)
{
    return atomic_load_explicit(&index->head, memory_order_relaxed);
}


inline __attribute__((always_inline)) int
al_append_inline(struct Atomic_List_Index *const index, _Atomic(void *)*const node, const bool is_sti)
{
    int ret = 0;
    if (is_sti) {
        __asm__ volatile ("cli"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    // memory_order_release: let *node write visible
    _Atomic(void *)*const old_end = atomic_exchange_explicit(&index->end, node, memory_order_relaxed);
    if (old_end == NULL)
        atomic_store_explicit(&index->head, node, memory_order_relaxed);
    else {
        atomic_store_explicit(old_end, (void *)node, memory_order_relaxed);
        ret = 1;
    }
    if (is_sti) {
        __asm__ ("sti"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    return ret;
}

inline __attribute__((always_inline)) int
al_append_empty_inline(struct Atomic_List_Index *const index, _Atomic(void *) *const node, const bool is_sti)
{
    int ret = 0;
    if (is_sti) {
        __asm__ ("cli"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    _Atomic(void *)* end = *(void *volatile *)&index->end;
    //_Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    // memory_order_release: let *node = NULL visible
    if (end == NULL && atomic_compare_exchange_strong_explicit(&index->end, &end, node, memory_order_relaxed, memory_order_relaxed))
        atomic_store_explicit(&index->head, node, memory_order_relaxed);
    else
        ret = 1;
    if (is_sti) {
        __asm__ ("sti"
                :"+m"(__not_exist_global_sym_for_asm_seq)
                :
                :);
    }
    return ret;
}

inline struct RET_al_delete_front al_delete_front(struct Atomic_List_Index *const index)
{
    struct RET_al_delete_front ret;
    _Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    if (end == NULL) {
        ret.head = ret.next = NULL;
        return ret;
    }
    while ((ret.head = atomic_load_explicit(&index->head, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause");
    if (ret.head == end) {
        // 链表中仅有一个元素
        *(void **)&index->head = NULL;  // not atomic write
        // memory_order_release: let index->head write visible
        if (!atomic_compare_exchange_strong_explicit(&index->end, &end, NULL, memory_order_release, memory_order_relaxed))
            goto label2;
        if (*(void **)ret.head != NULL)
            __builtin_unreachable();
        ret.next = NULL;
    } else {
label2:;
        // 链表中有多个元素
        while ((ret.next = atomic_load_explicit(ret.head, memory_order_relaxed)) == NULL)
            __asm__ volatile ("pause");
        *(void **)&index->head = ret.next;  // not atomic write
    }
    return ret;
}

inline struct RET_al_delete_front al_delete_front2(struct Atomic_List_Index *const index)
{
    struct RET_al_delete_front ret;
    _Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    if (end == NULL) {
        ret.head = ret.next = NULL;
        return ret;
    }
    while ((ret.head = atomic_load_explicit(&index->head, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause");
    if (ret.head == end) {
        // 链表中仅有一个元素
        atomic_store_explicit(&index->head, NULL, memory_order_relaxed);
        // memory_order_release: let index->head write visible
        if (!atomic_compare_exchange_strong_explicit(&index->end, &end, NULL, memory_order_release, memory_order_relaxed))
            goto label2;
        if (*(void **)ret.head != NULL)
            __builtin_unreachable();
        ret.next = NULL;
    } else {
label2:;
        // 链表中有多个元素
        while ((ret.next = atomic_load_explicit(ret.head, memory_order_relaxed)) == NULL)
            __asm__ volatile ("pause");
        atomic_store_explicit(&index->head, ret.next, memory_order_relaxed);
    }
    return ret;
}

inline struct RET_al_delete_front al_delete_front_force(struct Atomic_List_Index *const index)
{
    struct RET_al_delete_front ret;
    while ((ret.head = atomic_load_explicit(&index->head, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause");
    _Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    if (ret.head == end) {
        // 链表中仅有一个元素
        *(void **)&index->head = NULL;  // not atomic write
        // memory_order_release: let index->head write visible
        if (!atomic_compare_exchange_strong_explicit(&index->end, &end, NULL, memory_order_release, memory_order_relaxed))
            goto label2;
        if (*(void **)ret.head != NULL)
            __builtin_unreachable();
        ret.next = NULL;
    } else {
label2:;
        // 链表中有多个元素
        while ((ret.next = atomic_load_explicit(ret.head, memory_order_relaxed)) == NULL)
            __asm__ volatile ("pause");
        *(void **)&index->head = ret.next;  // not atomic write
    }
    return ret;
}

inline struct RET_al_delete_front al_delete_front2_force(struct Atomic_List_Index *const index)
{
    struct RET_al_delete_front ret;
    while ((ret.head = atomic_load_explicit(&index->head, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause");
    _Atomic(void *)* end = atomic_load_explicit(&index->end, memory_order_relaxed);
    if (ret.head == end) {
        // 链表中仅有一个元素
        atomic_store_explicit(&index->head, NULL, memory_order_relaxed);
        // memory_order_release: let index->head write visible
        if (!atomic_compare_exchange_strong_explicit(&index->end, &end, NULL, memory_order_release, memory_order_relaxed))
            goto label2;
        if (*(void **)ret.head != NULL)
            __builtin_unreachable();
        ret.next = NULL;
    } else {
label2:;
        // 链表中有多个元素
        while ((ret.next = atomic_load_explicit(ret.head, memory_order_relaxed)) == NULL)
            __asm__ volatile ("pause");
        atomic_store_explicit(&index->head, ret.next, memory_order_relaxed);
    }
    return ret;
}


inline struct RET_al_clear al_clear(struct Atomic_List_Index *const index)
{
    struct RET_al_clear ret;
    ret.end = atomic_load_explicit(&index->end, memory_order_relaxed);
    if (ret.end == NULL)
        return ret;
    while ((ret.head = atomic_load_explicit(&index->head, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause");
    *(void **)&index->head = NULL;
    // memory_order_release: let list->head write visible
    ret.end = atomic_exchange_explicit(&index->end, NULL, memory_order_release);
    return ret;
}
