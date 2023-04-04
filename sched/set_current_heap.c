#include "myos_sched.h"

#include "sched-internal.h"

#include <threads.h>

void set_current_heap(mi_heap_t* const heap)
{
    thrd_current()->_mi_heap_default = heap;
}
