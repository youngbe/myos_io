#include "boot.h"

#define E820_ENRTY_MAX 512

noreturn void bootloader_main()
{
    // 禁用 8259a
    disable_8259a();
    
    static struct E820_Entry e820_entrys[E820_ENRTY_MAX];
    size_t e820_entrys_num = detect_memory_e820(e820_entrys, E820_ENRTY_MAX);
    e820_entrys_num = trim_e820_entrys(e820_entrys, e820_entrys_num, e820_entrys);
    for (size_t i = 0; i < e820_entrys_num; ++i)
        printb("%u:addr:%llu, size:%llu, type: %u, extend: %u\n", i, e820_entrys[i].addr, e820_entrys[i].size, e820_entrys[i].type, e820_entrys[i].extend);
    
    void *const kernel_load_pos = find_kernel_load_pos(e820_entrys, e820_entrys_num);
    printb("loading kernel at: %p\n", kernel_load_pos);
    load_kernel(kernel_load_pos);

    enter64(kernel_load_pos, (uintptr_t)e820_entrys, e820_entrys_num, (uintptr_t)_ebootloader_align4k);
}
