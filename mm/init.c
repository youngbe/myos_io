#include "mm-internal.h"

#include <io.h>

#include <utlist.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <stdalign.h>
#include <stdatomic.h>

extern const unsigned char _kernel[];
extern alignas(0x200000) const unsigned char __virtual_null[];
extern unsigned char _ekernel[];
extern alignas(8) unsigned char _ekernel_align8[];
extern alignas(0x200000) unsigned char _ekernel_align2m[];

struct E820_Entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t extend;
};
struct Free_Memory_Block
{
    uintptr_t addr;
    size_t size;
    struct Free_Memory_Block *next;
    struct Free_Memory_Block *prev;
};

#define FREE_MEMORY_BLOCKS_MAX 2048
#define FIRST_PT_PAGE_V_ADDR ((uintptr_t)(0x40000000000 - 0x200000))

static struct PT_Page *free_pt_pages = (struct PT_Page *)(uintptr_t)FIRST_PT_PAGE_V_ADDR;
// init here
//static size_t free_pts_num;
static struct PT_Page *pt_pages_sp = (struct PT_Page *)(uintptr_t)FIRST_PT_PAGE_V_ADDR;
// init here
//static uintptr_t pt_pages_sp_min;

static const void **const free_pages = (const void **)(uintptr_t)_ekernel_align8;
// init here
//static _Atomic(size_t) free_pages_num;

static void map_in_first_pt_page(struct PT_Page* first_pt_page, const void* ph_addr, const void* v_addr);

struct Free_Pages_Iterator
{
    const struct Free_Memory_Block *block;
    uintptr_t addr;
    uintptr_t addr_max;
};
static void init_free_pages_iterator(struct Free_Pages_Iterator* iterator, const struct Free_Memory_Block* head);
static const void *get_free_page_in_init(void *in_parm);

#define FREE_PAGES_V_ADDR_MAX 0x10000000000 // 1T

void kernel_init_part2(const struct E820_Entry* const e820_entrys, const size_t e820_entrys_num)
{
    // 检查内核是否被加载到 16M - 4G 的位置上
    if ((uintptr_t)_kernel < 0x1000000 || (uintptr_t)_ekernel > 0x100000000) {
        fputs("kernel is not load in [16M, 4G] !\n", stderr);
        abort();
    }
    // 检查内核加载位置是否是对齐2M
    if (((uintptr_t)__virtual_null & 0x1fffff) != 0) {
        fputs("kernel is not align 2m !\n", stderr);
        abort();
    }
    const uintptr_t _kernel_align2m = (uintptr_t)_kernel & -0x200000;

    struct Free_Memory_Block blocks_buf[FREE_MEMORY_BLOCKS_MAX];
    size_t memory_blocks_alloced_num = 0;
    // 整理e820_entrys->blocks ，成为一个链表
    // 整理链表，将0 - 16M, 内核部分标记
    struct Free_Memory_Block *blocks = NULL;
    if (e820_entrys_num == 1 && e820_entrys[0].size == 0) {
        if (e820_entrys[0].type != 1 || e820_entrys[0].extend != 1)
            goto label_nomem;
        // 一个超级块
        if (_kernel_align2m != 0x1000000) {
            if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                fputs("too many memory blocks!\n", stderr);
                abort();
            }
            blocks_buf[memory_blocks_alloced_num].addr = 0x1000000;
            blocks_buf[memory_blocks_alloced_num].size = _kernel_align2m - 0x1000000;
            DL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
            ++memory_blocks_alloced_num;
        }

        if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
            fputs("too many memory blocks!\n", stderr);
            abort();
        }
        blocks_buf[memory_blocks_alloced_num].addr = (uintptr_t)_ekernel_align2m;
        blocks_buf[memory_blocks_alloced_num].size = UINTPTR_MAX - (uintptr_t)_ekernel_align2m + 1;
        DL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
        ++memory_blocks_alloced_num;
    }
    else {
        for (size_t i = 0; i < e820_entrys_num; ++i) {
            if (e820_entrys[i].type != 1 || e820_entrys[i].extend != 1)
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
            if ((uintptr_t)_kernel >= e820_entrys[i].addr && (uintptr_t)_ekernel_align2m - addr <= size) {
                // 包含内核
                if (_kernel_align2m > addr) {
                    if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                        fputs("too many memory blocks!\n", stderr);
                        abort();
                    }
                    blocks_buf[memory_blocks_alloced_num].addr = addr;
                    blocks_buf[memory_blocks_alloced_num].size = _kernel_align2m - addr;
                    DL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                    ++memory_blocks_alloced_num;
                }

                if ((uintptr_t)_ekernel_align2m - addr != size) {
                    if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                        fputs("too many memory blocks!\n", stderr);
                        abort();
                    }
                    blocks_buf[memory_blocks_alloced_num].addr = (uintptr_t)_ekernel_align2m;
                    blocks_buf[memory_blocks_alloced_num].size = size - ((uintptr_t)_ekernel_align2m - addr);
                    DL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                    ++memory_blocks_alloced_num;
                }
            } else {
                // 不包含内核
                if (memory_blocks_alloced_num == FREE_MEMORY_BLOCKS_MAX) {
                    fputs("too many memory blocks!\n", stderr);
                    abort();
                }
                blocks_buf[memory_blocks_alloced_num].addr = addr;
                blocks_buf[memory_blocks_alloced_num].size = size;
                DL_APPEND(blocks, &blocks_buf[memory_blocks_alloced_num]);
                ++memory_blocks_alloced_num;
            }
        }
    }


    // 在0-512G空间内找到一张空闲页，制作页表，对上述标记页进行映射
    {
        if (blocks == NULL || blocks->addr >= 0x8000000000) {
label_nomem:
            fputs("can't find free page!\n", stderr);
            abort();
        }
        struct PT_Page *const first_pt_page = (struct PT_Page *)blocks->addr;
        if (blocks->size == 0x200000)
            DL_DELETE(blocks, blocks);
        else {
            blocks->addr += 0x200000;
            blocks->size -= 0x200000;
        }


        // 进行映射
        init_pt_page(first_pt_page, (uintptr_t)first_pt_page);
        first_pt_page->prev = (struct PT_Page *)(uintptr_t)FIRST_PT_PAGE_V_ADDR;
        first_pt_page->next = NULL;
        // 映射内核
        for (uintptr_t addr = _kernel_align2m; addr < (uintptr_t)_ekernel; addr += 0x200000)
            map_in_first_pt_page(first_pt_page, (void *)addr, (void *)addr);
        // 映射页表
        map_in_first_pt_page(first_pt_page, first_pt_page, (void *)FIRST_PT_PAGE_V_ADDR); // 4T - 2M
        free_pts_num = first_pt_page->free_num;
    }

    // 切换cr3
    atomic_signal_fence(memory_order_release);
    __asm__ volatile(
            "movq   %1, %%cr3"
            :"+m"(__not_exist_global_sym_for_asm_seq)
            :"r"(&kernel_pt1)
            :);
    atomic_signal_fence(memory_order_acquire);

    // 建立空闲页栈
    {
        size_t free_pages_num_in_init = 0;
        struct Free_Pages_Iterator free_pages_iterator;
        init_free_pages_iterator(&free_pages_iterator, blocks);
        while (true) {
            const void *const free_page = get_free_page_in_init(&free_pages_iterator);
            if (free_page == NULL)
                break;
            // 获取到一张空闲页
            if (((uintptr_t)&free_pages[free_pages_num_in_init] & 0x1fffff) == 0) {
                const void *const free_page_for_array = get_free_page_in_init(&free_pages_iterator);
                if (free_page_for_array == NULL) {
                    add_pt_page(free_page, --pt_pages_sp);
                    break;
                }
                // 尝试进行map
                if (pt_try_map(free_page_for_array, &free_pages[free_pages_num_in_init], get_free_page_in_init, &free_pages_iterator, false)) {
                    add_pt_page(free_page, --pt_pages_sp);
                    add_pt_page(free_page_for_array, --pt_pages_sp);
                    break;
                }
            }
            free_pages[free_pages_num_in_init++] = free_page;
            if ((uintptr_t)&free_pages[free_pages_num_in_init] == FREE_PAGES_V_ADDR_MAX) {
                puts("Warning: too many physical memory, trim some ...");
                break;
            }
        }
        pt_pages_sp_min = (uintptr_t)&free_pages[free_pages_num_in_init];
        atomic_store_explicit(&free_pages_num, free_pages_num_in_init, memory_order_relaxed);
    }

    if (mtx_init(&mmap_mutex, mtx_plain) != thrd_success) {
        fputs("failed to init mutex for mmap!\n", stderr);
        abort();
    }
}

static void map_in_first_pt_page(struct PT_Page* const first_pt_page, const void* const ph_addr, const void* const v_addr)
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
        pt_dec_free_entrys_num(pt3);
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
        // 在页表映射之后才能读写映射区域
        // 防止编译器将读写映射区域的语句移到映射之前
        atomic_signal_fence(memory_order_acq_rel);
    }
    pt3[i3] = (uintptr_t)ph_addr | ((1 << 0) | (1 << 1) | (1 << 7));
    // 在页表映射之后才能读写映射区域
    // 防止编译器将读写映射区域的语句移到映射之前
    atomic_signal_fence(memory_order_acq_rel);
}

static void init_free_pages_iterator(struct Free_Pages_Iterator *iterator, const struct Free_Memory_Block *const head)
{
    iterator->block = head;
    if (head != NULL) {
        iterator->addr = head->addr;
        iterator->addr_max = head->addr + head->size;
    }
}

static const void *get_free_page_in_init(void *const in_parm)
{
    struct Free_Pages_Iterator *const it = in_parm;
    if (it->block == NULL)
        return NULL;
    void *const ret = (void *)it->addr;
    const uintptr_t new_addr = (uintptr_t)ret + 0x200000;
    if (new_addr == it->addr_max) {
        it->block = it->block->next;
        if (it->block != NULL) {
            it->addr = it->block->addr;
            it->addr_max = it->block->addr + it->block->size;
        }
    } else
        it->addr = new_addr;
    return ret;
}
