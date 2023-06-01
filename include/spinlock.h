#pragma once

#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <stddef.h>


// 使用方法：
// 创建Spin_Mutex, 使用函数或默认值初始化
//
// 创建Spin_Mutex_Member
// 使用函数或默认值初始化
// lock
// unlock , 如果需要再次使用这个Spin_Mutex_Member上锁，请重新初始化

struct Spin_Mutex_Member
{
    alignas(16) uint32_t is_locked;
    struct Spin_Mutex_Member *next;
};
typedef struct Spin_Mutex_Member spin_mtx_member_t;
typedef struct Spin_Mutex_Member *spin_mtx_t;

#define SPIN_MUTEX_INIT_VAL NULL
#define SPIN_MUTEX_MEMBER_INIT_VAL {0, NULL}

void cli_spinlock_do(void);
void spinlock_do(void);

static inline void spin_mutex_init(spin_mtx_t *const spin_mutex)
{
    *spin_mutex = NULL;
}

static inline void __attribute__((always_inline)) spin_mutex_member_init(struct Spin_Mutex_Member *const spin_mutex_member)
{
    spin_mutex_member->is_locked = 0;
    spin_mutex_member->next = NULL;
}

static inline void __attribute__((always_inline)) spin_lock_inline(spin_mtx_t *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    struct Spin_Mutex_Member *const last_last = atomic_exchange_explicit((volatile _Atomic(__typeof__(*spin_mutex)) *)spin_mutex, spin_mutex_member, memory_order_relaxed);
    if (last_last == NULL)
        return;
    atomic_store_explicit((_Atomic(__typeof__(last_last->next)) *)&last_last->next, spin_mutex_member, memory_order_relaxed);
    while (*(volatile uint32_t *)&spin_mutex_member->is_locked == 0)
        __asm__ volatile("pause":::);
}

static inline void __attribute__((always_inline)) spin_unlock_inline(spin_mtx_t *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    //spin_mtx_t temp = atomic_load_explicit((volatile _Atomic(spin_mtx_t) *)spin_mutex, memory_order_relaxed);
    spin_mtx_t temp = *(struct Spin_Mutex_Member *volatile *)spin_mutex;
    if (temp == spin_mutex_member && atomic_compare_exchange_strong_explicit((volatile _Atomic(spin_mtx_t) *)spin_mutex, &temp, NULL, memory_order_release, memory_order_relaxed)) {
        if (spin_mutex_member->next != NULL)
            __builtin_unreachable();
        return;
    }
    while ((temp = atomic_load_explicit((volatile _Atomic(__typeof__(spin_mutex_member->next)) *)&spin_mutex_member->next, memory_order_relaxed)) == NULL)
        __asm__ volatile ("pause":::);
    atomic_store_explicit((_Atomic(__typeof__(temp->is_locked)) *)&temp->is_locked, 1, memory_order_release);
}
