#include <threads.h>

int cnd_init(struct Cond *const cond)
{
    al_index_init(&cond->threads);
    spin_mutex_init(&cond->spin_mtx);
    return thrd_success;
}
