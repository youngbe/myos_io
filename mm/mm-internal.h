#pragma once

#include <stdint.h>
#include <stdalign.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <threads.h>

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

// 只是初始化 struct PT_Page* pt_page 中的成员变量，而不做其他任何事情
// 第二个参数是该PT_Page的物理地址
static void init_pt_page(struct PT_Page* pt_page, uintptr_t ph_addr);
// 新增一个PT_Page到页表池中
static void add_pt_page(const void* free_ph_page, struct PT_Page *new_pt_page_v_addr);
// 对该pt的空闲条目--
static void pt_dec_free_entrys_num(const uint64_t* pt);
// 对该pt的空闲条目++，如果pt空了,则释放它
// return true == 空了
static bool pt_inc_free_entrys_num_and_free_pt(const uint64_t* pt);
// map 物理地址->虚拟地址，总是成功，如果无法映射，则abort()
// 如果该虚拟地址已经被映射，则abort()
static void pt_map_always_success(const void* ph_addr, void* v_addr);
// map 物理地址->虚拟地址
// 第三个参数：如果页表不足以映射，使用该函数申请新的物理页作为PT_Page
// 第四个参数：用于第三个参数的调用参数
// is_soft：如果为真，则如果该位置已经被映射，则不进行映射并正常返回
// 如果为假，则如果该位置已经被映射，则abort()
static int pt_try_map(const void* ph_addr, void* v_addr, const void* (*get_free_page_hook)(void *), void* parm, bool is_soft);
// unmap 虚拟地址，返回物理地址
// 如果该位置没有map，则abort()
static const void* pt_unmap(void* v_addr);
static bool pt_is_map(const void* v_addr);

alignas(4096) const uint64_t kernel_pt1[512];
static alignas(4096) uint64_t kernel_pt2s[64][512];
static void* kernel_vpt2s[64][512];

static struct PT_Page* free_pt_pages;
static size_t free_pts_num;
static struct PT_Page* pt_pages_sp;
static uintptr_t pt_pages_sp_min;

static const void**const free_pages;
static _Atomic(size_t) free_pages_num;

// 当拥有这把锁时，可以读写 上面六个变量
// 当没有这把锁时，只能原子读取 free_pages_num
static mtx_t mmap_mutex;
