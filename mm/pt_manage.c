#include "mm-internal.h"

#include <utlist.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

static void init_pt_page(struct PT_Page *const pt_page, const uintptr_t ph_addr)
{
    memset(pt_page->page_tables, 0, sizeof(pt_page->page_tables));
    pt_page->ph_addr = ph_addr;
    pt_page->free_num = 510;
    for (size_t i = 0; i < 510; ++i)
        pt_page->free_ids[i] = 509 - i;
}
static void add_pt_page(const void *const free_ph_page, struct PT_Page *const new_pt_page_v_addr)
{
    pt_map_always_success(free_ph_page, new_pt_page_v_addr);
    init_pt_page(new_pt_page_v_addr, (uintptr_t)free_ph_page);
    DL_APPEND(free_pt_pages, new_pt_page_v_addr);
    free_pts_num += 510;
}
static void pt_dec_free_entrys_num(const uint64_t *const pt)
{
    const size_t id = ((uintptr_t)pt & 0x1fffff) >> 12;
    struct PT_Page *const pt_page = (struct PT_Page *)((uintptr_t)pt & -0x200000);
    --pt_page->free_entrys_num[id];
}
static bool pt_inc_free_entrys_num_and_free_pt(const uint64_t *const pt)
{
    const size_t id = ((uintptr_t)pt & 0x1fffff) >> 12;
    struct PT_Page *const pt_page = (struct PT_Page *)((uintptr_t)pt & -0x200000);
    if (++pt_page->free_entrys_num[id] == 512) {
        // 这个pt空了，释放它
        pt_page->free_ids[pt_page->free_num++] = id;
        if (pt_page->free_num == 1)
            DL_PREPEND(free_pt_pages, pt_page);
        ++free_pts_num;
        return true;
    }
    return false;
}
static void pt_map_always_success(const void *const ph_addr, void *const v_addr)
{
    const size_t i1 = ((uintptr_t)v_addr >> 39) & 0x1ff;
    const size_t i2 = ((uintptr_t)v_addr >> 30) & 0x1ff;
    const size_t i3 = ((uintptr_t)v_addr >> 21) & 0x1ff;
    uint64_t *pt3;
    if (kernel_vpt2s[i1][i2] != NULL) {
        pt3 = kernel_vpt2s[i1][i2];
        if (pt3[i3] != 0) {
            fputs("remap a page!\n", stderr);
            abort();
        }
        pt_dec_free_entrys_num(pt3);
    }
    else {
        // alloc a new pt3
        const uint16_t id = free_pt_pages->free_ids[--free_pt_pages->free_num];
        free_pt_pages->free_entrys_num[id] = 511;
        pt3 = free_pt_pages->page_tables[id];
        kernel_vpt2s[i1][i2] = pt3;
        kernel_pt2s[i1][i2] = ((uintptr_t)pt3 & 0x1fffff) + free_pt_pages->ph_addr + ((1 << 0) | (1 << 1));
        // 在页表映射之后才能读写映射区域
        // 防止编译器将读写映射区域的语句移到映射之前
        atomic_signal_fence(memory_order_acq_rel);
        if (free_pt_pages->free_num == 0)
            DL_DELETE(free_pt_pages, free_pt_pages);
        --free_pts_num;
    }
    pt3[i3] = (uintptr_t)ph_addr | ((1 << 0) | (1 << 1) | (1 << 7));
    // 在页表映射之后才能读写映射区域
    // 防止编译器将读写映射区域的语句移到映射之前
    atomic_signal_fence(memory_order_acq_rel);
}
static int pt_try_map(const void *const ph_addr, void *const v_addr, const void* (*const get_free_page_hook)(void *), void *const parm, const bool is_soft)
{
    const size_t i1 = ((uintptr_t)v_addr >> 39) & 0x1ff;
    const size_t i2 = ((uintptr_t)v_addr >> 30) & 0x1ff;
    const size_t i3 = ((uintptr_t)v_addr >> 21) & 0x1ff;
    uint64_t *pt3;
    // 检查是否需要新的pt
    if (kernel_vpt2s[i1][i2] != NULL) {
        // 不需要新的pt
        pt3 = kernel_vpt2s[i1][i2];
        if (pt3[i3] != 0) {
            if (is_soft)
                return 1;
            fputs("remap a page!\n", stderr);
            abort();
        }
        pt_dec_free_entrys_num(pt3);
    }
    else {
        // 需要新的pt
        if (free_pts_num == 1) {
            // 剩下最后一个free_pt，用来映射新的PT_Page
            if ((uintptr_t)pt_pages_sp <= pt_pages_sp_min || (uintptr_t)pt_pages_sp - pt_pages_sp_min < sizeof(struct PT_Page))
                return -1;
            const void *const new_free_ph_page = get_free_page_hook(parm);
            if (new_free_ph_page == NULL)
                return -1;
            add_pt_page(new_free_ph_page, --pt_pages_sp);
        }
        // 从free_pt_pages中获取一个新pt
        const uint16_t id = free_pt_pages->free_ids[--free_pt_pages->free_num];
        free_pt_pages->free_entrys_num[id] = 511;
        pt3 = free_pt_pages->page_tables[id];
        kernel_vpt2s[i1][i2] = pt3;
        kernel_pt2s[i1][i2] = ((uintptr_t)pt3 & 0x1fffff) + free_pt_pages->ph_addr + ((1 << 0) | (1 << 1));
        // 在页表映射之后才能读写映射区域
        // 防止编译器将读写映射区域的语句移到映射之前
        atomic_signal_fence(memory_order_acq_rel);
        if (free_pt_pages->free_num == 0)
            DL_DELETE(free_pt_pages, free_pt_pages);
        --free_pts_num;
    }
    pt3[i3] = (uintptr_t)ph_addr | ((1 << 0) | (1 << 1) | (1 << 7));
    // 在页表映射之后才能读写映射区域
    // 防止编译器将读写映射区域的语句移到映射之前
    atomic_signal_fence(memory_order_acq_rel);
    return 0;
}

static const void* pt_unmap(void *const v_addr)
{
    const size_t i1 = ((uintptr_t)v_addr >> 39) & 0x1ff;
    const size_t i2 = ((uintptr_t)v_addr >> 30) & 0x1ff;
    const size_t i3 = ((uintptr_t)v_addr >> 21) & 0x1ff;

    uint64_t *pt3 = kernel_vpt2s[i1][i2];

    if (pt3[i3] == 0)
        abort();

    const void *const ph_addr = (const void *)(uintptr_t)(pt3[i3] & (uintptr_t)-0x200000);

    // 在页表解除映射之前需完成对映射区域的读写
    // 防止编译器将读写映射区域的语句移到解除映射之后
    atomic_store_explicit((_Atomic(uint64_t) *)&pt3[i3], 0, memory_order_release);
    if (pt_inc_free_entrys_num_and_free_pt(pt3)) {
        kernel_vpt2s[i1][i2] = NULL;
        atomic_store_explicit((_Atomic(uint64_t) *)&kernel_pt2s[i1][i2], 0, memory_order_release);
    }

    return ph_addr;
}

static bool pt_is_map(const void* const v_addr)
{
    const size_t i1 = ((uintptr_t)v_addr >> 39) & 0x1ff;
    const size_t i2 = ((uintptr_t)v_addr >> 30) & 0x1ff;
    const size_t i3 = ((uintptr_t)v_addr >> 21) & 0x1ff;
    uint64_t *pt3;
    if (kernel_vpt2s[i1][i2] != NULL) {
        pt3 = kernel_vpt2s[i1][i2];
        return pt3[i3] != 0;
    }
    else
        return false;
}
