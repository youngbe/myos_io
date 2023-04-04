#pragma once

#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>


// 使用方法：
// 创建Spin_Mutex, 使用函数或默认值初始化
//
// 创建Spin_Mutex_Member
// 使用函数或默认值初始化
// lock
// unlock , 如果需要再次使用这个Spin_Mutex_Member上锁，请先destroy，后初始化
// 在Spin_Mutex_Member变量被内存回收前，对使用该变量lock过的Mutex进行destroy

struct Spin_Mutex_Member
{
    alignas(16) uint32_t is_locked;
};

struct Spin_Mutex
{
    struct Spin_Mutex_Member *owner;
    struct Spin_Mutex_Member *last;
    struct Spin_Mutex_Member _default;
};

#define SPIN_MUTEX_INIT(x) {&x._default, NULL, {0}}
#define SPIN_MUTEX_MEMBER_INIT(x) {1}

static inline void spin_mutex_init(struct Spin_Mutex *const spin_mutex)
{
    spin_mutex->last = &spin_mutex->_default;
    spin_mutex->_default.is_locked = 0;
}

static inline void spin_mutex_member_init(struct Spin_Mutex_Member *const spin_mutex_member)
{
    spin_mutex_member->is_locked = 1;
}

static inline void spin_lock(struct Spin_Mutex *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    // 为了更好的性能，下面三句话需要连在一起
    // 使用memory_order_release的原因：保证对member的初始化已写入
    struct Spin_Mutex_Member *const watching = atomic_exchange_explicit((volatile _Atomic(__typeof__(spin_mutex->last)) *)&spin_mutex->last, spin_mutex_member, memory_order_release);
    while (atomic_load_explicit((volatile _Atomic(__typeof__(watching->is_locked)) *)&watching->is_locked, memory_order_acquire) != 0) {}
    atomic_store_explicit((_Atomic(__typeof__(spin_mutex->owner)) *)&spin_mutex->owner, spin_mutex_member, memory_order_relaxed);
}

static inline void spin_unlock(struct Spin_Mutex_Member *const spin_mutex_member)
{
    atomic_store_explicit((_Atomic(__typeof__(spin_mutex_member->is_locked)) *)&spin_mutex_member->is_locked, 0, memory_order_release);
}

static inline void spin_mutex_member_destroy(struct Spin_Mutex *const spin_mutex, struct Spin_Mutex_Member *const spin_mutex_member)
{
    struct Spin_Mutex_Member *temp = spin_mutex_member;
    // 如果成功了，则代表没有排队者，spin_unlock是否写入没有关系；但是，如果成功了且spin_unlock未写入，则也会开锁（需要刷新写入）
    // 如果失败了，说明有排队者，需要保证spin_unlock已经写入
    atomic_signal_fence(memory_order_acq_rel);
    if (atomic_compare_exchange_strong_explicit((volatile _Atomic(__typeof__(spin_mutex->last)) *)&spin_mutex->last, &temp, &spin_mutex->_default, memory_order_relaxed, memory_order_relaxed))
        return;
    // 等待后面的线程将owner抢走后可以free(spin_mutex_member)
    while (atomic_load_explicit((volatile _Atomic(__typeof__(spin_mutex->owner)) *)&spin_mutex->owner, memory_order_relaxed) == spin_mutex_member) {}
}
