#include "mm.h"
#include "mm-internal.h"

#include <utlist.h>

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>


static const void* get_free_page_generic(void *parm)
{
    size_t *p_free_pages_num = parm;
    if (*p_free_pages_num == 0)
        return NULL;
    return free_pages[--*p_free_pages_num];
}

static int __attribute__((noinline)) alloc_page_lock_routine(void *addr, const size_t num, const bool is_soft)
{
    struct Mapped_Page
    {
        struct Mapped_Page* next;
        struct Mapped_Page* prev;
    };
    struct Mapped_Page* mapped_pages = NULL;

    if (mtx_lock(&mmap_mutex) != thrd_success)
        abort();
    atomic_signal_fence(memory_order_acq_rel);
    size_t _free_pages_num = atomic_load_explicit(&free_pages_num, memory_order_relaxed);

    for (size_t i = 0; i != num; ++i, addr = (void *)((uintptr_t)addr + 0x200000)) {
        if (_free_pages_num == 0)
            goto label_failed;
        const void *const new_free_page = free_pages[--_free_pages_num];
        const int ret = pt_try_map(new_free_page, addr, get_free_page_generic, &_free_pages_num, is_soft);
        if (ret < 0) {
            free_pages[_free_pages_num++] = new_free_page;
            goto label_failed;
        }
        else if (ret == 0)
            CDL_PREPEND(mapped_pages, (struct Mapped_Page *)addr);
    }

    atomic_store_explicit(&free_pages_num, _free_pages_num, memory_order_relaxed);
    atomic_signal_fence(memory_order_acq_rel);
    if (mtx_unlock(&mmap_mutex) != thrd_success)
        abort();
    return 0;

label_failed:
    ;
    struct Mapped_Page* page, *tmp;
    DL_FOREACH_SAFE(mapped_pages, page, tmp) {
        const void *const free_page = pt_unmap(page);
        free_pages[_free_pages_num++] = free_page;
    }
    atomic_store_explicit(&free_pages_num, _free_pages_num, memory_order_relaxed);
    atomic_signal_fence(memory_order_acq_rel);
    if (mtx_unlock(&mmap_mutex) != thrd_success)
        abort();
    return -1;
}

void *mmap(void *const addr, const size_t length, const int prot, const int flags, const int, const off_t offset)
{
    if (prot != (PROT_READ | PROT_WRITE | PROT_EXEC))
        abort();
    if (flags != (MAP_PRIVATE | MAP_ANONYMOUS))
        abort();
    if (offset != 0)
        abort();
    if ((uintptr_t)addr % 0x200000 != 0)
        abort();
    if (length % 0x200000 != 0)
        abort();

    // 检查是否符合内核地址空间
    if ((uintptr_t)addr < 0x40000000000 || (uintptr_t)addr > 0x200000000000)
        abort();
    if (0x200000000000 - (uintptr_t)addr < length)
        abort();

    const size_t alloc_pages_num = length / 0x200000;
    if (alloc_pages_num > atomic_load_explicit(&free_pages_num, memory_order_relaxed))
        return (void *)-1;
    if (alloc_pages_num == 0)
        return addr;

    if (alloc_page_lock_routine(addr, alloc_pages_num, false) == 0) {
        memset(addr, 0, length);
        return addr;
    }
    else
        return (void *)-1;
}

void *mmap_soft(void *const addr, const size_t length, const int prot, const int flags, const int, const off_t offset)
{
    if (prot != (PROT_READ | PROT_WRITE | PROT_EXEC))
        abort();
    if (flags != (MAP_PRIVATE | MAP_ANONYMOUS))
        abort();
    if (offset != 0)
        abort();
    if ((uintptr_t)addr % 0x200000 != 0)
        abort();
    if (length % 0x200000 != 0)
        abort();

    // 检查是否符合内核地址空间
    if ((uintptr_t)addr < 0x40000000000 || (uintptr_t)addr > 0x200000000000)
        abort();
    if (0x200000000000 - (uintptr_t)addr < length)
        abort();

    const size_t alloc_pages_num = length / 0x200000;
    if (alloc_pages_num > atomic_load_explicit(&free_pages_num, memory_order_relaxed))
        return (void *)-1;
    if (alloc_pages_num == 0)
        return addr;

    if (alloc_page_lock_routine(addr, alloc_pages_num, true) == 0)
        return addr;
    else
        return (void *)-1;
}

static void __attribute__((noinline)) free_page_lock_routine(void* v_addr, size_t num)
{
    if (mtx_lock(&mmap_mutex) != thrd_success)
        abort();
    atomic_signal_fence(memory_order_acq_rel);
    size_t _free_pages_num = atomic_load_explicit(&free_pages_num, memory_order_relaxed);

    while (true) {
        const void *const free_page = pt_unmap(v_addr);
        free_pages[_free_pages_num++] = free_page;
        if (--num == 0)
            break;
        v_addr = (void *)((uintptr_t)v_addr + 0x200000);
    }

    atomic_store_explicit(&free_pages_num, _free_pages_num, memory_order_relaxed);
    atomic_signal_fence(memory_order_acq_rel);
    if (mtx_unlock(&mmap_mutex) != thrd_success)
        abort();
}

int munmap(void *addr, size_t length)
{
    if ((uintptr_t)addr % 0x200000 != 0)
        abort();
    if (length % 0x200000 != 0)
        abort();

    // 检查是否符合内核地址空间
    if ((uintptr_t)addr < 0x40000000000 || (uintptr_t)addr > 0x200000000000)
        abort();
    if (0x200000000000 - (uintptr_t)addr < length)
        abort();

    const size_t need_free_pages_num = length / 0x200000;
    if (need_free_pages_num == 0)
        return 0;

    free_page_lock_routine(addr, need_free_pages_num);
    return 0;
}

static int __attribute__((noinline)) check_map_lock_routine(const void* v_addr, size_t num)
{
    if (mtx_lock(&mmap_mutex) != thrd_success)
        abort();
    atomic_signal_fence(memory_order_acq_rel);

    const bool is_map = pt_is_map(v_addr);
    while (true) {
        if (pt_is_map(v_addr) != is_map)
            goto label_part_map;
        if (--num == 0)
            break;
        v_addr = (void *)((uintptr_t)v_addr + 0x200000);
    }

    atomic_signal_fence(memory_order_acq_rel);
    if (mtx_unlock(&mmap_mutex) != thrd_success)
        abort();

    int ret;
    if (is_map)
        ret = MAP_ALL_MAPPED;
    else
        ret = MAP_NO_MAPPED;
    return ret;

label_part_map:
    atomic_signal_fence(memory_order_acq_rel);
    if (mtx_unlock(&mmap_mutex) != thrd_success)
        abort();
    return MAP_PART_MAPPED;
}

int check_map(const void *const addr, const size_t length)
{
    if ((uintptr_t)addr % 0x200000 != 0)
        abort();
    if (length % 0x200000 != 0)
        abort();

    // 检查是否符合内核地址空间
    if ((uintptr_t)addr < 0x40000000000 || (uintptr_t)addr > 0x200000000000)
        abort();
    if (0x200000000000 - (uintptr_t)addr < length)
        abort();

    const size_t check_pages_num = length / 0x200000;
    if (check_pages_num == 0)
        abort();

    return check_map_lock_routine(addr, check_pages_num);
}

size_t estimate_free_mem(void)
{
    return atomic_load_explicit(&free_pages_num, memory_order_relaxed) * 0x200000;
}
