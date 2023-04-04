#include "init.h"

#include <stdio.h>
#include <stdlib.h>

static int new_pt_page(void *const parm)
{
    struct Free_Pages_Iterator *const free_pages_iterator = parm;
    const void *const new_free_page = get_free_page(free_pages_iterator);
    if (new_free_page == NULL)
        return -1;
    add_pt_page(new_free_page);
    return 0;
}

noreturn void kernel_init_part1(const struct Free_Memory_Block *const blocks, const struct E820_Entry *const e820_entrys, const size_t e820_entrys_num)
{
    // 建立空闲页栈，从4T的位置开始
    struct Free_Pages_Iterator free_pages_iterator;
    if (init_free_pages_iterator(&free_pages_iterator, blocks)) {
        fputs("no free pages!\n", stderr);
        abort();
    }
    while (true) {
        const void *const free_page = get_free_page(&free_pages_iterator);
        if (free_page == NULL)
            break;
        // 获取到一张空闲页
        if (((uintptr_t)&free_pages[free_pages_num] & 0x1fffff) == 0) {
            const void *const free_page_for_array = get_free_page(&free_pages_iterator);
            if (free_page_for_array == NULL) {
                add_pt_page(free_page);
                break;
            }
            // 尝试进行map
            if (try_map(free_page_for_array, &free_pages[free_pages_num], new_pt_page, &free_pages_iterator, false)) {
                add_pt_page(free_page);
                add_pt_page(free_page_for_array);
                break;
            }
        }
        free_pages[free_pages_num++] = free_page;
        if ((uintptr_t)&free_pages[free_pages_num] == FREE_PAGES_V_ADDR_MAX) {
            puts("Warning: too many physical memory, trim some ...");
            break;
        }
    }
    pt_pages_sp_min = (uintptr_t)&free_pages[free_pages_num];

    mmap_init();
    // 现在可以使用mmap() munmap()了

    const size_t current_core_id = get_core_id();
    {
        extern struct Thread main_thread;
        extern void mi_process_load(void);

        // 此局部变量需要保留至 threads 被真正地初始化
        struct Thread* p_main_thread;

        // 虚假地初始化threads，让get_current_thread()可以被临时使用
        p_main_thread = &main_thread;
        threads = &p_main_thread - current_core_id;
        mi_process_load();
        // 现在可以使用 malloc() free() 了

        /*
        // 对 threads 进行正确的初始化
        const size_t cores_num = get_cores_num();
        const size_t core_id_max = get_core_id_max();
        const size_t core_id_min = get_core_id_min();
        assert(cores_num >= 1);
        assert(core_id_max >= core_id_min);
        assert(cores_num - 1 <= core_id_max - core_id_min);

        if (SIZE_MAX / sizeof(struct Thread *) - 1 < (cores_id_max - core_id_min)) {
            fputs("too many cores!", stderr);
            abort("too many cores!");
        }
        assert((cores_id_max - core_id_min + 1) * sizeof(struct Thread *) <= SIZE_MAX);
        if (((core_id_max - core_id_min) >> 4) > cores_num)
            printf("Warning: core_id_max - core_id_min > 16 * cores_num, the array of cores maybe much larger than cores_num !\n");

        struct Thread **threads_buf = (struct Thread **)malloc((cores_id_max - core_id_min + 1) * sizeof(struct Thread *)) - core_id_min;
        if (threads_buf == NULL) {
            fputs("malloc array for threads failed! Maybe too many cores !\n", stderr);
            abort();
        }
        static_assert((uintptr_t)NULL == 0);
        memset(threads_buf, 0, (cores_id_max - core_id_min + 1) * sizeof(struct Thread *));
        threads_buf[get_core_id()] = &main_thread;
        threads = threads_buf;
        */
    }

    size_t all_free_memory = 0;
    for (size_t i = 0; i < e820_entrys_num; ++i) {
        if ((e820_entrys[i].type == 1 || e820_entrys[i].type == 2) && e820_entrys[i].extend == 1)
            all_free_memory += e820_entrys[i].size;
    }
    printf("All free memory: %lx\n", all_free_memory);
    printf("Init free memory: %lu * 2M\n", free_pages_num);


    __asm__ volatile("":::"memory");
    while (true) {}
    __builtin_unreachable();
}
