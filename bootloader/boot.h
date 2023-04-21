#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <assert.h>
#include <string.h>
#include <stdalign.h>

#include "io.h"

#ifndef __WORDSIZE
# error "__WORDSIZE is not defined!"
#endif
#if __WORDSIZE != 32
# error "this file must be 32 bit"
#endif

static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

// tty.c
void console_init(void);
void __putstr(const char *);

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

// disable_8259a.c
void disable_8259a(void);

// E820
struct E820_Entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t extend;
};
size_t detect_memory_e820(struct E820_Entry *const e820_entrys, const size_t limit);
size_t trim_e820_entrys(const struct E820_Entry *const in_entrys, const size_t in_entrys_num, struct E820_Entry *const out_entrys);
#define PHY_ADDR_MAX 0xffffffffffffffff

// LOAD KERNEL
void *find_kernel_load_pos(const struct E820_Entry *entrys, size_t num);
void load_kernel(void *pos);

// enter64.s
noreturn void enter64(void *start64, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// main.c
noreturn void bootloader_main();

noreturn static inline void bootloader_error(const char *const str)
{
    __putstr("Error: ");
    __putstr(str);
    while(true) {}
    __builtin_unreachable();
}

// provided by bootloader.ld
extern unsigned char _ebootloader[];
extern alignas(16) unsigned char _ebootloader_align16[];
extern alignas(512) unsigned char _ebootloader_align512[];
extern alignas(4096) unsigned char _ebootloader_align4k[];

static_assert(KERNEL_SIZE <= 0x100000000 - 0x1000000, "Kernel too large!");
#define KERNEL_SIZE_ALIGN512 (((size_t)(KERNEL_SIZE) + 0x1ff) & -0x200)
#define KERNEL_SIZE_ALIGN2M (((size_t)(KERNEL_SIZE) + 0x1fffff) & -0x200000)

// acpi
ssize_t map_keyboard_interrupt_to_vector(uint8_t apic_id, uint8_t vector);
