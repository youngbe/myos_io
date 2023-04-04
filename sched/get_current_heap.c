#include "myos_sched.h"

#include "sched-internal.h"

#include <threads.h>

mi_heap_t* get_current_heap(void)
{
    return thrd_current()->_mi_heap_default;
}
