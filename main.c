#include <stdio.h>
#include <stdnoreturn.h>
#include <threads.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <io.h>

noreturn void kernel_main()
{
    puts("init finished, kernel started!");

    char str[200];
    while (true) {
        printf(":#");
	fflush(stdout);
        scanf("%[^\n]%*c", str);
        printf("get common: %s\n", str);
    }

    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
