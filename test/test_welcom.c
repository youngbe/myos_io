#include <stdio.h>
#include <stdnoreturn.h>
#include <stdbool.h>


noreturn void kernel_main()
{
    for (size_t i = 0; i < 25; ++i)
        putchar('\n');
    puts("============================================================");
    puts(" ++++          ++++ ++       ++    ++++++        ++++++     ");
    puts(" ++ ++        ++ ++  ++     ++    +++  +++     +++    +++   ");
    puts(" ++ ++        ++ ++   ++   ++    ++      ++  +++        +++ ");
    puts(" ++  ++      ++  ++    ++ ++    ++        ++   +++          ");
    puts(" ++   ++    ++   ++     +++     ++        ++     ++++++     ");
    puts(" ++   ++    ++   ++     +++     ++        ++          +++   ");
    puts(" ++    ++  ++    ++     +++      ++      ++  +++        +++ ");
    puts(" ++     ++++     ++     +++       +++  +++     +++    +++   ");
    puts(" ++     ++++     ++     +++        ++++++        ++++++     ");
    puts("============================================================");
    putchar('\n');
    puts("Welcome to MYOS!");
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');
    putchar('\n');



    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
