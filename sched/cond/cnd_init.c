#include <threads.h>

int cnd_init(cnd_t *const cnd)
{
    cnd->threads = NULL;
    spin_mutex_init(&cnd->spin_mtx);
    return thrd_success;
}
