#pragma once

#include "kernel.h"

#include <utlist.h>

#include <stdint.h>
#include <stddef.h>
#include <stdnoreturn.h>
#include <stdalign.h>
#include <assert.h>

// symbols privided by linker
extern int __virtual_null[];
extern int _ekernel[];
extern int _ekernel_align2m[];

// memory_blocks.c
struct E820_Entry
{
    uint64_t addr;
    uint64_t size;
    uint32_t type;
    uint32_t extend;
};

// Free_Memory_Block
#define FREE_MEMORY_BLOCKS_MAX 2048
struct Free_Memory_Block
{
    uintptr_t addr;
    size_t size;
    struct Free_Memory_Block *next;
    struct Free_Memory_Block *prev;
};
struct Free_Pages_Iterator
{
    const struct Free_Memory_Block *block;
    const struct Free_Memory_Block *head;
    uintptr_t addr;
    uintptr_t addr_max;
};
int init_free_pages_iterator(struct Free_Pages_Iterator *iterator, const struct Free_Memory_Block *head);
const void *get_free_page(struct Free_Pages_Iterator *);


// 来自内核模块
// page table page: 2M Memory page only save page table
struct PT_Page
{
    // page_table 有两种类型： virtual_page_table(vpt)：保存虚拟地址的页表
    // phy_page_table(pt)：保存物理地址的页表
    // 内核地址空间：预设pt1, pt2, vpt2，存在pt3
    // 用户态地址空间：存在pt1 pt2 pt3 vpt1 vpt2
    alignas(4096) uint64_t page_tables[510][512];
    // 此PT_Page的物理地址
    uintptr_t ph_addr;
    // 此PT_Page还有多少个空闲pt
    uint16_t free_num;
    // 空闲pt的索引数组
    uint16_t free_ids[510];
    // 将空闲的PT_Page(free_num != 0) 连成链表
    struct PT_Page *prev;
    struct PT_Page *next;
    // 每个pt还有多少空闲条目，只对以下pt生效：
    //  1. 用户态 pt2 pt3
    //  2. 内核态 pt3
    uint16_t free_entrys_num[510];
    // 从虚拟pt到物理pt的索引
    // 只对用户态vpt1 vpt2生效
    // pptp: phy page table's pointer
    void *pptp[510];
};
static_assert(sizeof(struct PT_Page) == 0x200000);

#define FIRST_PT_PAGE_V_ADDR (0x40000000000 - 0x200000) // 4T - 2M
extern size_t free_pts_num;
extern uintptr_t pt_pages_sp_min;

#define FREE_PAGES_V_ADDR_MAX 0x10000000000 // 1T
extern const void *free_pages[];
extern size_t free_pages_num;

void init_pt_page(struct PT_Page * pt_page, uintptr_t ph_addr);
void add_pt_page(const void* free_ph_page);
void dec_free_entrys_num(const uint64_t* pt);
int try_map(const void *ph_addr, const void *v_addr, int (*new_pt_page_p)(void *), void *parm, bool is_soft);

extern alignas(4096) const uint64_t kernel_pt1[512];
extern alignas(4096) uint64_t kernel_pt2s[64][512];
extern void *kernel_vpt2s[64][512];

void mmap_init(void);


// stdio.c
void stdio_init(void);

// sched.h
// mimalloc.h
struct mi_heap_s;
typedef struct mi_heap_s mi_heap_t;
struct Thread
{
    size_t id;
    mi_heap_t* _mi_heap_default;
    int __errno;
};
extern struct Thread** threads;
static inline size_t get_core_id()
{

    uint32_t apicid;
    __asm__ volatile
        (
         "rdmsr"
         :"=a"(apicid)
         :"c"((uint32_t)0x802)
         :
         );
    return (size_t)apicid;
}


// init.c
noreturn void kernel_init_part0(const struct E820_Entry *e820_entrys, size_t entrys_num);
noreturn void kernel_init_part1(const struct Free_Memory_Block *blocks, const struct E820_Entry *e820_entrys, size_t entrys_num);
