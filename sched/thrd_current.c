#include <threads.h>

#include "thrd_current.h"

thrd_t thrd_current(void)
{
    return thrd_currentx();
}
