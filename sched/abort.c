#include <stdlib.h>

#include <io.h>

#include <stdnoreturn.h>
#include <stdbool.h>

noreturn void abort_handler(void);

noreturn void abort(void)
{
    __asm__ volatile("cli":"+m"(__not_exist_global_sym_for_asm_seq)::);
    // 向其它处理器发送abort ipi
    wrmsr_volatile_seq(0x830, 34 | 0b000 << 8 | 1 << 14 | 0b11 << 18);

    abort_handler();
}
