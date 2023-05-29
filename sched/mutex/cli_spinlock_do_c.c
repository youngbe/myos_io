#include <spinlock.h>

struct RET
{
    unsigned long x;
    unsigned long y;
};

struct RET cli_spinlock_do(
    struct RET (*func)(void *, void *, void *, void *, void *, void *),
    spin_mtx_t *spin_mutex, void *arg1, void *arg2, void *arg3, void *arg4)
{
    void *val;
    __asm__ volatile ("":::"r10", "r11");
    struct RET ret;
    struct Spin_Mutex_Member spin_mutex_member;
    spin_mutex_member_init(&spin_mutex_member);
    spin_lock_inline(spin_mutex, &spin_mutex_member);
    ret = func(val, spin_mutex, arg1, arg2, arg3, arg4);
    spin_unlock_inline(spin_mutex, &spin_mutex_member);
    return ret;
}
