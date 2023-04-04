#include "types.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

//#define StrongAlias(TargetFunc, AliasDecl)  \
//  extern __typeof__ (TargetFunc) AliasDecl  \
//    __attribute__ ((alias (#TargetFunc), copy (TargetFunc)));

//#define MI_FORWARD(fun)      __attribute__((alias(#fun), used, visibility("default")));

int putc(const int c, FILE *const fp)
{
    return ((struct Struct_FILE *)fp)->putchar(c);
}

int fputc(const int c, FILE *const fp) __attribute__((alias("putc"), used, visibility("default")));
//StrongAlias (putc, fputc);

int putchar(const int c)
{
    return ((struct Struct_FILE *)stdout)->putchar(c);
}

int puts(const char *const str)
{
    return ((struct Struct_FILE *)stdout)->putline(str);
}

int fputs(const char *const str, FILE *const fp)
{
    return ((struct Struct_FILE *)fp)->putstr(str);
}

size_t fwrite(const void* buf, const size_t size, const size_t nmemb, FILE* const fp)
{
    if (size == 0 || nmemb == 0)
        return 0;
    size_t ret = 0;
    while (true) {
        size_t request;
        if ((nmemb - ret) > SIZE_MAX / size) {
            const size_t nwb = SIZE_MAX / size;
            request = nwb * size;
        }
        else
            request = size * (nmemb - ret);
        const size_t write_count = ((struct Struct_FILE *)fp)->write(buf, request);
        ret += write_count / size;
        if (request != write_count || ret == nmemb)
            break;
        buf = (const void *)((uintptr_t)buf + write_count);
    }
    return ret;
}

int printf(const char *fmt, ...)
{
    char printf_buf[4096];
    va_list args;
    int printed;

    va_start(args, fmt);
    printed = vsprintf(printf_buf, fmt, args);
    va_end(args);

    fputs(printf_buf, stdout);

    return printed;
}
