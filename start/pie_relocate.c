#include <elf.h>

#include <stdint.h>
#include <assert.h>

extern unsigned char __virtual_null[];

// 重定位
void pie_relocate()
{
    extern const Elf64_Rela __rela_dyn_start[];
    extern const Elf64_Rela __rela_dyn_end[];
    const Elf64_Rela* rela = __rela_dyn_start;

    while (rela != __rela_dyn_end) {
        assert(rela->r_info == R_X86_64_RELATIVE);
        *(void **)((uintptr_t)__virtual_null + rela->r_offset) = (void *)((uintptr_t)__virtual_null + rela->r_addend);
        ++rela;
    }
    __asm__ volatile ("":::"memory");
}
