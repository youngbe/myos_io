#include "init.h"

#include <stdlib.h>
#include <stdio.h>

static void map_in_first_pt_page(struct PT_Page *const first_pt_page, const void *const ph_addr, const void *const v_addr)
{
    const size_t i1 = ((uintptr_t)v_addr >> 39) & 0x1ff;
    const size_t i2 = ((uintptr_t)v_addr >> 30) & 0x1ff;
    const size_t i3 = ((uintptr_t)v_addr >> 21) & 0x1ff;
    uint64_t *pt3;
    if (kernel_pt2s[i1][i2] != 0) {
        pt3 = (uint64_t *)(uintptr_t)(kernel_pt2s[i1][i2] & -0x1000);
        if (pt3[i3] != 0) {
            fputs("remap a page!\n", stderr);
            abort();
        }
        dec_free_entrys_num(pt3);
    }
    else {
        // alloc a new pt3
        if (first_pt_page->free_num <= 1) {
            fputs("too many page to map!\n", stderr);
            abort();
        }
        const uint16_t id = first_pt_page->free_ids[--first_pt_page->free_num];
        first_pt_page->free_entrys_num[id] = 511;
        pt3 = first_pt_page->page_tables[id];
        kernel_vpt2s[i1][i2] = (void *)(FIRST_PT_PAGE_V_ADDR + ((uintptr_t)pt3 & 0x1fffff));
        kernel_pt2s[i1][i2] = (uintptr_t)pt3 + ((1 << 0) | (1 << 1));
    }
    pt3[i3] = (uintptr_t)ph_addr | ((1 << 0) | (1 << 1) | (1 << 7));
}

// 内核启动协议：
// 1. 进入内核时，已经进入64位长模式
// 2. 进入内核时，已经完成x86-64-v3的初始化
// 3. 进入内核时，已经加载64位段描述符表，64位段描述符表的位置保存在1M以下
// 4. 进入内核时，准备好0 - 512G的1：1映射页表，使用1G大页进行映射(只需两张页表即可(8K)，页表的地址在1M以下)
// 5. 内核被加载到 16M - 4G 的地址上，对齐2M
// 6. 传入e820获取的内存分布，以e820条目数组的形式进行传入，需要对条目进行整理：处理重叠、越界、并排序；传入的数组位置同样在1M以下
// 7. 进入内核时准备至少64K的栈，栈的位置在1M以下，注意不要和上述e820数组、页表、全局段描述符表位置重叠；进入内核时%rsp对齐16字节
noreturn void kernel_init_part0(const struct E820_Entry *const e820_entrys, const size_t e820_entrys_num)
{
    stdio_init();
    // 现在可以使用printf fprintf sprintf vsprintf
    // putc fputc putchar
    // puts fputs
    // stdout stderr

    // 当前实现puts返回值永远等于0，后续不再检查
    puts("kernel start initing!");

    // 检查内核是否被加载到 16M - 4G 的位置上
    if ((uintptr_t)__virtual_null < 0x1000000 || (uintptr_t)_ekernel > 0x100000000) {
        fputs("kernel is not load in [16M, 4G] !\n", stderr);
        abort();
    }
    // 检查内核加载位置是否是对齐2M
    if (((uintptr_t)__virtual_null & 0x1fffff) != 0) {
        fputs("kernel is not align 2m !\n", stderr);
        abort();
    }

    struct Free_Memory_Block blocks_buf[FREE_MEMORY_BLOCKS_MAX];
    size_t memory_blocks_alloced_num = 0;
    // 整理e820_entrys->blocks ，成为一个链表
    // 整理链表，将0 - 16M, 内核部分标记
    struct Free_Memory_Block *blocks = NULL;
    if (e820_entrys_num == 1 && e820_entrys[0].size == 0) {
        // 一个超级块
        if ((uintptr_t)__virtual_null != 0x1000000) {
            if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                fputs("too many memory blocks!\n", stderr);
                abort();
            }
            blocks_buf[memory_blocks_alloced_num].addr = 0x1000000;
            blocks_buf[memory_blocks_alloced_num].size = (uintptr_t)__virtual_null - 0x1000000;
            CDL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
            ++memory_blocks_alloced_num;
        }

        if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
            fputs("too many memory blocks!\n", stderr);
            abort();
        }
        blocks_buf[memory_blocks_alloced_num].addr = (uintptr_t)_ekernel_align2m;
        blocks_buf[memory_blocks_alloced_num].size = UINTPTR_MAX - (uintptr_t)_ekernel_align2m + 1;
        CDL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
        ++memory_blocks_alloced_num;
    }
    else {
        for (size_t i = 0; i < e820_entrys_num; ++i) {
            if ((e820_entrys[i].type != 1 && e820_entrys[i].type != 2) || e820_entrys[i].extend != 1)
                continue;
            size_t pad;
            if (e820_entrys[i].addr <= 0x1000000)
                pad = 0x1000000 - e820_entrys[i].addr;
            else
                pad = 0x200000 - (e820_entrys[i].addr & 0x1fffff);
            if (pad >= e820_entrys[i].size)
                continue;
            const size_t size = (e820_entrys[i].size - pad) & -0x200000;
            if (size == 0)
                continue;
            const uintptr_t addr = e820_entrys[i].addr + pad;
            if ((uintptr_t)__virtual_null >= addr && (uintptr_t)_ekernel_align2m - addr <= size) {
                // 包含内核
                if ((uintptr_t)__virtual_null != addr) {
                    if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                        fputs("too many memory blocks!\n", stderr);
                        abort();
                    }
                    blocks_buf[memory_blocks_alloced_num].addr = addr;
                    blocks_buf[memory_blocks_alloced_num].size = (uintptr_t)__virtual_null - addr;
                    CDL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                    ++memory_blocks_alloced_num;
                }

                if ((uintptr_t)_ekernel_align2m - addr != size) {
                    if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                        fputs("too many memory blocks!\n", stderr);
                        abort();
                    }
                    blocks_buf[memory_blocks_alloced_num].addr = (uintptr_t)_ekernel_align2m;
                    blocks_buf[memory_blocks_alloced_num].size = size - ((uintptr_t)_ekernel_align2m - addr);
                    CDL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                    ++memory_blocks_alloced_num;
                }
            }
            else {
                if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                    fputs("too many memory blocks!\n", stderr);
                    abort();
                }
                blocks_buf[memory_blocks_alloced_num].addr = addr;
                blocks_buf[memory_blocks_alloced_num].size = size;
                CDL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                ++memory_blocks_alloced_num;
            }
        }
    }


    // 在0-512G空间内找到一张空闲页，制作页表，对上述标记页进行映射
    if (blocks == NULL || blocks->addr >= 0x8000000000) {
        fputs("can't find free page!\n", stderr);
        abort();
    }
    struct PT_Page *const first_pt_page = (struct PT_Page *)blocks->addr;
    if (blocks->size == 0x200000)
        CDL_DELETE(blocks, blocks);
    else {
        blocks->addr += 0x200000;
        blocks->size -= 0x200000;
    }


    // 找1张空闲页标记，作为2M栈
    if (blocks == NULL) {
        fputs("can't find free page!\n", stderr);
        abort();
    }
    void *const stack_page = (void *)blocks->addr;
    if (blocks->size == 0x200000)
        CDL_DELETE(blocks, blocks);
    else {
        blocks->addr += 0x200000;
        blocks->size -= 0x200000;
    }



    // 进行映射
    init_pt_page(first_pt_page, (uintptr_t)first_pt_page);
    first_pt_page->next = first_pt_page->prev = (struct PT_Page *)(uintptr_t)FIRST_PT_PAGE_V_ADDR;
    // 映射内核
    for (uintptr_t addr = (uintptr_t)__virtual_null; addr < (uintptr_t)_ekernel; addr += 0x200000)
        map_in_first_pt_page(first_pt_page, (void *)addr, (void *)addr);
    // 映射页表
    map_in_first_pt_page(first_pt_page, first_pt_page, (void *)FIRST_PT_PAGE_V_ADDR); // 4T - 2M
    // 映射栈
    map_in_first_pt_page(first_pt_page, stack_page, (void *)(uintptr_t)(0x200000000000 - 0x200000)); // 32T - 2M
    
    free_pts_num = first_pt_page->free_num;



    // 切换页表和栈
    __asm__ volatile(
            "movq   $0x200000000000, %%rsp\n\t" // 32T
            "movq   %0, %%cr3\n\t"
            "callq  kernel_init_part1"
            :
            :"r"((uintptr_t)kernel_pt1), "X"(kernel_init_part1), "D"(blocks), "S"(e820_entrys), "d"(e820_entrys_num)
            :"rsp", "memory");
    __builtin_unreachable();
}
