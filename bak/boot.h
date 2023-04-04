#pragma once

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

// wordsize.h
#if defined __x86_64__ && !defined __ILP32__
# define __WORDSIZE 64
#else
# define __WORDSIZE 32
#define __WORDSIZE32_SIZE_ULONG     0
#define __WORDSIZE32_PTRDIFF_LONG   0
#endif

#ifdef __x86_64__
# define __WORDSIZE_TIME64_COMPAT32 1
/* Both x86-64 and x32 use the 64-bit system call interface.  */
# define __SYSCALL_WORDSIZE     64
#else
# define __WORDSIZE_TIME64_COMPAT32 0
#endif


#if __WORDSIZE != 32
ERROR: this file must be 32 bit
#endif

// stddef.h
typedef unsigned int size_t;
typedef signed int ssize_t;
#ifdef __cplusplus
#  if !defined(__MINGW32__) && !defined(_MSC_VER)
#    define NULL __null
#  else
#    define NULL 0
#  endif
#else
#  define NULL ((void*)0)
#endif
#ifdef __cplusplus
#if defined(_MSC_EXTENSIONS) && defined(_NATIVE_NULLPTR_SUPPORTED)
namespace std { typedef decltype(nullptr) nullptr_t; }
using ::std::nullptr_t;
#endif
#endif

#if __has_builtin(__builtin_offsetof)
#  define offsetof(TYPE, MEMBER) __builtin_offsetof (TYPE, MEMBER)
#else
#  define offsetof(type, member) ((size_t)&((type*)0)->member)
#endif

// GCC
//#ifdef __GNUG__
//#define NULL __null
//#else   /* G++ */
//#ifndef __cplusplus
//#define NULL ((void *)0)
//#else   /* C++ */
//#define NULL 0
//#endif  /* C++ */
//#endif  /* G++ */


// limits.h
/* Number of bits in a `char'.	*/
#  define CHAR_BIT	8

/* Minimum and maximum values a `signed char' can hold.  */
#  define SCHAR_MIN	(-128)
#  define SCHAR_MAX	127

/* Maximum value an `unsigned char' can hold.  (Minimum is 0.)  */
#  define UCHAR_MAX	255

/* Minimum and maximum values a `char' can hold.  */
#  ifdef __CHAR_UNSIGNED__
#   define CHAR_MIN	0
#   define CHAR_MAX	UCHAR_MAX
#  else
#   define CHAR_MIN	SCHAR_MIN
#   define CHAR_MAX	SCHAR_MAX
#  endif

/* Minimum and maximum values a `signed short int' can hold.  */
#  define SHRT_MIN	(-32768)
#  define SHRT_MAX	32767

/* Maximum value an `unsigned short int' can hold.  (Minimum is 0.)  */
#  define USHRT_MAX	65535

/* Minimum and maximum values a `signed int' can hold.  */
#  define INT_MIN	(-INT_MAX - 1)
#  define INT_MAX	2147483647

/* Maximum value an `unsigned int' can hold.  (Minimum is 0.)  */
#  define UINT_MAX	4294967295U

/* Minimum and maximum values a `signed long int' can hold.  */
#  if __WORDSIZE == 64
#   define LONG_MAX	9223372036854775807L
#  else
#   define LONG_MAX	2147483647L
#  endif
#  define LONG_MIN	(-LONG_MAX - 1L)

/* Maximum value an `unsigned long int' can hold.  (Minimum is 0.)  */
#  if __WORDSIZE == 64
#   define ULONG_MAX	18446744073709551615UL
#  else
#   define ULONG_MAX	4294967295UL
#  endif

/* Minimum and maximum values a `signed long long int' can hold.  */
#   define LLONG_MAX	9223372036854775807LL
#   define LLONG_MIN	(-LLONG_MAX - 1LL)

/* Maximum value an `unsigned long long int' can hold.  (Minimum is 0.)  */
#   define ULLONG_MAX	18446744073709551615ULL

// stdint.h
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;

typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;

typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;

/* Types for `void *' pointers.  */
#ifndef __intptr_t_defined
typedef int			intptr_t;
# define __intptr_t_defined
#endif
typedef unsigned int		uintptr_t;

// stdbool.h
#define __bool_true_false_are_defined 1

#if __STDC_VERSION__ > 201710L
/* FIXME: We should be issuing a deprecation warning here, but cannot yet due
 * to system headers which include this header file unconditionally.
 */
#elif !defined(__cplusplus)
#define bool _Bool
#define true 1
#define false 0
#elif defined(__GNUC__) && !defined(__STRICT_ANSI__)
/* Define _Bool as a GNU extension. */
#define _Bool bool
#if __cplusplus < 201103L
/* For C++98, define bool, false, true as a GNU extension. */
#define bool bool
#define false false
#define true true
#endif
#endif

// stdarg.h
typedef __builtin_va_list va_list;
#define va_start(v, l)	__builtin_va_start(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v, T)	__builtin_va_arg(v, T)
#define va_copy(d, s)	__builtin_va_copy(d, s)

static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static inline void outb(const uint8_t val, const uint16_t port)
{
    __asm__ volatile(
            "outb   %%al, %%dx"
            :
            :"a"(val),"d"(port)
            :);
}

static inline uint8_t inb(const uint16_t port)
{
    uint8_t val;
    __asm__ volatile(
            "inb    %%dx, %%al"
            :"=a"(val)
            :"d"(port)
            :);
    return val;
}

// tty.c
void __putstr(const char *);

// tty_i386.c
void console_init_i386(void);
void __putstr_i386(const char *);

// string.c && ffreestanding
size_t strnlen(const char *s, size_t maxlen);
void *memmove(void *dest, const void *src, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

// printb.c
int sprintb(char *buf, const char *fmt, ...);
int vsprintb(char *buf, const char *fmt, va_list args);
int printb(const char *fmt, ...);

/* bioscall.c */
struct biosregs {
	union {
		struct {
			uint32_t edi;
			uint32_t esi;
			uint32_t ebp;
			uint32_t _esp;
			uint32_t ebx;
			uint32_t edx;
			uint32_t ecx;
			uint32_t eax;
			uint32_t _fsgs;
			uint32_t _dses;
			uint32_t eflags;
		};
		struct {
			uint16_t di, hdi;
			uint16_t si, hsi;
			uint16_t bp, hbp;
			uint16_t _sp, _hsp;
			uint16_t bx, hbx;
			uint16_t dx, hdx;
			uint16_t cx, hcx;
			uint16_t ax, hax;
			uint16_t gs, fs;
			uint16_t es, ds;
			uint16_t flags, hflags;
		};
		struct {
			uint8_t dil, dih, edi2, edi3;
			uint8_t sil, sih, esi2, esi3;
			uint8_t bpl, bph, ebp2, ebp3;
			uint8_t _spl, _sph, _esp2, _esp3;
			uint8_t bl, bh, ebx2, ebx3;
			uint8_t dl, dh, edx2, edx3;
			uint8_t cl, ch, ecx2, ecx3;
			uint8_t al, ah, eax2, eax3;
		};
	};
};
void intcall(uint8_t int_no, const struct biosregs *ireg, struct biosregs *oreg);

// regs.c
static inline void initregs(struct biosregs *reg)
{
    memset(reg, 0, sizeof(*reg));
    // CF FIXED IF
    reg->eflags = 1 | (1 << 1) | (1 << 9);
}

// E820
struct E820_Entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t extend;
};
