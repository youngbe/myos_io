#pragma once

#include <stddef.h>
#include <stdint.h>

struct Thread;
typedef struct Thread* thrd_t;
struct Spin_Mutex_Member;
typedef struct Spin_Mutex_Member *spin_mtx_t;

struct Mutex
{
    // 可以原子地存/取owner，实现无锁trylock
    _Atomic(struct Thread*) owner;
    // count只会被owner修改，因此不用担心抢占问题
    size_t count;

    struct Thread* blocked_threads;
    // 这个自旋锁有两个，作用：
    // 1. 持有这把自旋锁时，才能对blocked_threads进行读写
    // 2. 如果owner是本线程（说明这个Mutex属于本线程），只有在持有这把自旋锁的状态下，才能将owner改为NULL。（这意味着，如果一个线程抢到了这个自旋锁，发现这个Mutex的owner不是本线程也不是NULL，则在放开这个自旋锁前，owner不会变成NULL（但有可能别成其他线程的owner））
    spin_mtx_t spin_mtx;
};
typedef struct Mutex mtx_t;

enum
{
  thrd_success  = 0,
  thrd_busy     = 1,
  thrd_error    = 2,
  thrd_nomem    = 3,
  thrd_timedout = 4
};
enum
{
  mtx_plain     = 0,
  mtx_recursive = 1,
  mtx_timed     = 2
};

struct Cond
{
    struct Thread* threads;
    spin_mtx_t spin_mtx;
};
typedef struct Cond cnd_t;

thrd_t thrd_current(void);

// 注意：mtx_init,mtx_destroy 对 mtx_lock,mtx_trylock,mtx_unlock 不是线程安全的
int mtx_init(mtx_t *mtx, int type);
void mtx_destroy(mtx_t *mtx);

int mtx_lock(mtx_t *mtx);
int mtx_trylock(mtx_t *mtx);
int mtx_unlock(mtx_t *mtx);

int cnd_init(cnd_t *cond);
int cnd_broadcast(cnd_t *cond);
int cnd_wait(cnd_t *cond, mtx_t *mtx);
int cnd_signal(cnd_t *cond);

typedef int (* thrd_start_t)(void*);
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
//[[noreturn]] void thrd_exit(int res);
