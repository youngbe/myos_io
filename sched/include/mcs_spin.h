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

typedef struct Spin_Mutex_Member *spin_mtx_t;
/*struct Spin_Mutex
{
    struct Spin_Mutex_Member *last;
};
*/

#define SPIN_MUTEX_INIT_VAL NULL
#define SPIN_MUTEX_MEMBER_INIT_VAL {0, NULL}

static inline void spin_mutex_init(spin_mtx_t *const spin_mutex)
{
    *spin_mutex = NULL;
}

static inline void spin_mutex_member_init(struct Spin_Mutex_Member *const spin_mutex_member)
{
    spin_mutex_member->is_locked = 0;
    spin_mutex_member->next = NULL;
}

static inline void spin_lock(spin_mtx_t *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    // 为保证性能，这几句需要连着一起执行

    // 使用memory_order_release的原因：保证对member的初始化已写入
    struct Spin_Mutex_Member *const last_last = atomic_exchange_explicit((volatile _Atomic(__typeof__(*spin_mutex)) *)spin_mutex, spin_mutex_member, memory_order_release);
    if (last_last == NULL) {
        // 如果成功了，就是上锁了
        atomic_signal_fence(memory_order_acquire);
        return;
    }
    atomic_store_explicit((_Atomic(__typeof__(last_last->next)) *)&last_last->next, spin_mutex_member, memory_order_relaxed);
    // 需要保证上面一句执行后才能执行下一句
    atomic_signal_fence(memory_order_acq_rel);
    //while (atomic_load_explicit((volatile _Atomic(__typeof__(spin_mutex_member->is_locked)) *)&spin_mutex_member->is_locked, memory_order_acquire) == 0) {}
    /*
    while (atomic_load_explicit((volatile _Atomic(__typeof__(spin_mutex_member->is_locked)) *)&spin_mutex_member->is_locked, memory_order_acquire) == 0)
        __asm__ volatile("");
        */
    __asm__ volatile (
            ".p2align	4, 0x90\n"
            "1:\n\t"
            "cmpl   %0, %1\n\t"
            "jne    1f\n\t"
            "pause\n\t"
            "jmp    1b\n"
            "1:"
            :"+m"(spin_mutex_member->is_locked)
            :"r"((uint32_t)0)
            :);
    atomic_signal_fence(memory_order_acquire);
}

static inline void __attribute__((always_inline)) spin_unlock(spin_mtx_t *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    spin_mtx_t temp = spin_mutex_member;
    if (atomic_compare_exchange_strong_explicit((volatile _Atomic(spin_mtx_t) *)spin_mutex, &temp, NULL, memory_order_release, memory_order_relaxed)) {
        if (spin_mutex_member->next != NULL)
            __builtin_unreachable();
        return;
    }
    //while ((temp = atomic_load_explicit((volatile _Atomic(__typeof__(spin_mutex_member->next)) *)&spin_mutex_member->next, memory_order_relaxed)) == NULL) {}
    __asm__ volatile (
            ".p2align	4, 0x90\n"
            "1:\n\t"
            "movq   %1, %0\n\t"
            "testq  %0, %0\n\t"
            "jne    1f\n\t"
            "pause\n\t"
            "jmp    1b\n"
            "1:"
            :"=r"(temp), "+m"(spin_mutex_member->next)
            :
            :);
    atomic_store_explicit((_Atomic(__typeof__(temp->is_locked)) *)&temp->is_locked, 1, memory_order_release);
}
