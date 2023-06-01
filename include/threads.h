#pragma once

#include <al_list.h>
#include <spinlock.h>

#include <stddef.h>
#include <stdint.h>

struct Thread;
typedef struct Thread* thrd_t;

struct Mutex
{
    _Atomic(struct Thread *) owner;
    _Atomic(void *) waiters;
    _Atomic(void *) wait_end;
    //al_index_t threads;
    size_t count;
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
    al_index_t threads;
    spin_mtx_t spin_mutex;
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
