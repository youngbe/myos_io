#include <stdnoreturn.h>
#include <stdio.h>

/*
noreturn void kernel_abort_str(const char *const str)
{
    __putstr("Error: ");
    __putstr(str);
    while(true) {}
    __builtin_unreachable();
}
*/

noreturn void abort()
{
    fputs("Error: kernel abort!\n", stderr);
    while(true) {}
    __builtin_unreachable();
}
