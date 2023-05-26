// atomic_list
#include <stdatomic.h>
#include <stddef.h>

// atomic list element
typedef _Atomic(void *) al_el_t;

// atomic list
typedef struct 
{
    _Atomic(_Atomic(void *) *) head;
    _Atomic(_Atomic(void *) *) end;
    spin_mutex_t spin_mutex;
} al_t;

static inline 
void al_append(al_t *const list, al_el_t *const el);

inline 
void al_append(al_t *const list, _Atomic(void *)*const el)
{
    // *el = NULL
    *(void **)el = NULL;
    //atomic_store_explicit(el, NULL, memory_order_relaxed);
    asm ("cli");
    //cli();
    _Atomic(void *)*const old_end = atomic_exchange_explicit(&list->end, el, memory_order_relaxed);
    if (old_end == NULL) {
        atomic_store_explicit(&list->head, el, memory_order_relaxed);
        //list->head = el;
    } else {
        atomic_store_explicit(old_end, (void *)el, memory_order_relaxed);
        //*old_end = (void *)el;
    }
    asm ("sti");
    //sti();
}

static inline al_el_t* al_delete_front(al_t *const list);

inline _Atomic(void *)* al_delete_front(al_t *const list)
{
    _Atomic(void *)* ret;
    asm ("cli");
    spin_lock();
    _Atomic(void *)* end = atomic_load_explicit(&list->end, memory_order_relaxed);
    if (end == NULL) {
        ret = NULL;
label_unlock:
        spin_unlock();
        asm ("sti");
        return ret;
    }
    _Atomic(void *)* head;
    while ((head = atomic_load_explicit(&list->head, memory_order_relaxed)) == NULL)
        asm ("pause");
    if (head == end) {
        // list 中仅有一个元素
        *(void **)list->head = NULL;  // not atomic write
        // 先将 head 设置为 NULL，然后设置end 为 NULL
        if (!atomic_compare_exchange_strong_explicit(&list->end, &end, NULL, memory_order_relaxed, memory_order_relaxed))
            goto label2;
    } else {
label2:
        // list 中有多个元素
        void *next;
        while ((next = atomic_load_explicit(head, memory_order_relaxed)) == NULL)
            asm ("pause");
        *(void **)list->head = next;  // not atomic write
    }
    ret = head;
    goto label_unlock;
}

struct RET_al_clear
{
    _Atomic(void *)* head;
    _Atomic(void *)* end;
};

struct RET_al_clear al_clear(al_t *const list)
{
    struct RET_al_clear ret;
    asm("cli");
    spin_lock();
    ret.end = atomic_load_explicit(&list->end, memory_order_relaxed);
    if (ret.end == NULL) {
label_unlock:
        spin_unlock();
        asm("sti");
        return ret;
    }
    while ((ret.head = atomic_load_explicit(&list->head, memory_order_relaxed)) == NULL)
        asm ("pause");
    *(void **)list->head = NULL;
    ret.end = atomic_exchange_explicit(&list->end, NULL, memory_order_relaxed);
    goto label_unlock;
}

#include <stdlib.h>
al_t list;

void test()
{
    _Atomic(void *)* p = calloc(800, 800);
    al_append(&list, p);
}
