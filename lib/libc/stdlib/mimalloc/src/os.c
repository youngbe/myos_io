/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE   // ensure mmap flags are defined
#endif

#include "mimalloc.h"
#include "mimalloc-internal.h"
#include "mimalloc-atomic.h"

#include <mm.h>

#include <string.h>  // strerror
#include <assert.h>

/* -----------------------------------------------------------
  Initialization.
  On windows initializes support for aligned allocation and
  large OS pages (if MIMALLOC_LARGE_OS_PAGES is true).
----------------------------------------------------------- */
bool _mi_os_decommit(void* addr, size_t size, mi_stats_t* stats);
bool _mi_os_commit(void* addr, size_t size, bool* is_zero, mi_stats_t* tld_stats);

static void* mi_align_up_ptr(void* p, size_t alignment) {
  return (void*)_mi_align_up((uintptr_t)p, alignment);
}

static void* mi_align_down_ptr(void* p, size_t alignment) {
  return (void*)_mi_align_down((uintptr_t)p, alignment);
}


// page size (initialized properly in `os_init`)
static const size_t os_page_size = 2*MI_MiB;

// if non-zero, use large page allocation
static const size_t large_os_page_size = 2*MI_MiB;

// is memory overcommit allowed?
// set dynamically in _mi_os_init (and if true we use MAP_NORESERVE)
static const bool os_overcommit = false;

bool _mi_os_has_overcommit(void) {
  return os_overcommit;
}

// OS (small) page size
size_t _mi_os_page_size(void) {
  return os_page_size;
}

// if large OS pages are supported (2 or 4MiB), then return the size, otherwise return the small page size (4KiB)
size_t _mi_os_large_page_size(void) {
  return (large_os_page_size != 0 ? large_os_page_size : _mi_os_page_size());
}

// round to a good OS allocation size (bounded by max 12.5% waste)
static size_t _mi_os_good_alloc_size(size_t size) {
  return _mi_align_up(size, os_page_size);
}

void _mi_os_init(void) {
  // get the page size
  // os_page_size = 2*MI_MiB;
  // os_alloc_granularity = 2*MI_MiB;
  // large_os_page_size = 2*MI_MiB;
  // os_overcommit = false;
}


/* -----------------------------------------------------------
  aligned hinting
-------------------------------------------------------------- */

// Return a MI_SEGMENT_SIZE aligned address that is probably available.
// If this returns NULL, the OS will determine the address but on some OS's that may not be
// properly aligned which can be more costly as it needs to be adjusted afterwards.
// For a size > 1GiB this always returns NULL in order to guarantee good ASLR randomization;
// (otherwise an initial large allocation of say 2TiB has a 50% chance to include (known) addresses
//  in the middle of the 2TiB - 6TiB address range (see issue #372))

#define MI_HINT_BASE ((uintptr_t)4 << 40)  // 4TiB start
#define MI_HINT_MAX  ((uintptr_t)32 << 40) // wrap after 32TiB
#define MI_ALIGN_MAX ((uintptr_t)4 << 30) // 4G

static_assert(MI_HINT_BASE % MI_ALIGN_MAX == 0);
static_assert(MI_ALIGN_MAX % MI_SEGMENT_SIZE == 0);
static_assert(MI_HINT_MAX % MI_ALIGN_MAX == 0);
static_assert(MI_HINT_MAX > MI_HINT_BASE);

static mi_decl_cache_align _Atomic(uintptr_t)aligned_base = MI_HINT_BASE;

static void* mi_os_get_aligned_hint(size_t try_alignment, size_t size)
{
    // 保证传入try_alignment不等于0且是2的幂
    mi_assert_internal((try_alignment != 0) && ((try_alignment & (try_alignment - 1)) == 0));
    mi_assert_internal(size != 0);

    {
        const size_t align_size = _mi_align_up(size, MI_SEGMENT_SIZE);
        // wrap !!
        if (align_size < size)
            return NULL;
        size = align_size;
    }
    if (size >= estimate_free_mem())
        return NULL;

    if (MI_SEGMENT_SIZE >= try_alignment) {
        uintptr_t hint = atomic_load_explicit(&aligned_base, memory_order_relaxed);
        if (MI_HINT_MAX - hint < size)
            return NULL;
        uintptr_t new_base = hint + size;
        while (!atomic_compare_exchange_strong_explicit(&aligned_base, &hint, new_base, memory_order_relaxed, memory_order_relaxed)) {
            if (MI_HINT_MAX - hint < size)
                return NULL;
            new_base = hint + size;
        }
        return (void*)hint;
    }
    else if (try_alignment > MI_ALIGN_MAX)
        return NULL;
    else {
        uintptr_t org = atomic_load_explicit(&aligned_base, memory_order_relaxed);
        uintptr_t hint = _mi_align_up(org, try_alignment);
        if (MI_HINT_MAX - hint < size)
            return NULL;
        uintptr_t new_base = hint + size;
        while (!atomic_compare_exchange_strong_explicit(&aligned_base, &org, new_base, memory_order_relaxed, memory_order_relaxed)) {
            hint = _mi_align_up(org, try_alignment);
            if (MI_HINT_MAX - hint < size)
                return NULL;
            new_base = hint + size;
        }
        return (void*)hint;
    }
}

/* -----------------------------------------------------------
  Free memory
-------------------------------------------------------------- */

static bool mi_os_mem_free(void* addr, size_t size, bool was_committed, mi_stats_t* stats)
{
  if (addr == NULL || size == 0) return true; // || _mi_os_is_huge_reserved(addr)
  bool err = false;
  if (was_committed) {
    err = (munmap(addr, size) == -1);
  }
  else {
      const int ret = check_map(addr, size);
      if (ret != MAP_NO_MAPPED)
          abort();
  }
  if (err) {
    _mi_warning_message("unable to release OS memory: addr: %p, size: %zu\n", addr, size);
  }
  if (was_committed) { _mi_stat_decrease(&stats->committed, size); }
  _mi_stat_decrease(&stats->reserved, size);
  return !err;
}


/* -----------------------------------------------------------
   Primitive allocation from the OS.
-------------------------------------------------------------- */

/* -----------------------------------------------------------
  OS API: alloc, free, alloc_aligned
----------------------------------------------------------- */

void* _mi_os_alloc_aligned(size_t size, size_t alignment, bool commit, bool* large, mi_stats_t* tld_stats);
void* _mi_os_alloc(size_t size, mi_stats_t* tld_stats) {
  MI_UNUSED(tld_stats);
  mi_stats_t* stats = &_mi_stats_main;
  if (size == 0) return NULL;
  size = _mi_os_good_alloc_size(size);
  return _mi_os_alloc_aligned(size, 0, true, NULL, stats);
}

void  _mi_os_free_ex(void* p, size_t size, bool was_committed, mi_stats_t* tld_stats) {
  MI_UNUSED(tld_stats);
  mi_stats_t* stats = &_mi_stats_main;
  if (size == 0 || p == NULL) return;
  size = _mi_os_good_alloc_size(size);
  mi_os_mem_free(p, size, was_committed, stats);
}

void  _mi_os_free(void* p, size_t size, mi_stats_t* stats) {
  _mi_os_free_ex(p, size, true, stats);
}

void* _mi_os_alloc_aligned(size_t size, size_t alignment, bool commit, bool* large, mi_stats_t* tld_stats)
{
    MI_UNUSED(tld_stats);

    // 修改前面调用此函数部分的代码，保证输入 size 对齐到页
    if (size == 0) return NULL;
    if (size % _mi_os_page_size() != 0) abort();

    // 原mimalloc代码能保证alignment是2的幂
    if (alignment <= _mi_os_page_size()) {
        if (alignment != 0 && _mi_os_page_size() % alignment != 0)
            abort();
        alignment = _mi_os_page_size();
    }
    else {
        if (alignment % _mi_os_page_size() != 0)
            abort();
    }

    void* p = mi_os_get_aligned_hint(alignment, size);
    if (p != NULL) {
        if (commit) {
            p = mmap(p, size, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (p == (void *)-1) {
                p = NULL;
            }
        }

        if (large)
            *large = false;
    }

    mi_stats_t*const stats = &_mi_stats_main;
    mi_stat_counter_increase(stats->mmap_calls, 1);
    if (p != NULL) {
        _mi_stat_increase(&stats->reserved, size);
        if (commit) { _mi_stat_increase(&stats->committed, size); }
    }
    return p;
}

/* -----------------------------------------------------------
  OS aligned allocation with an offset. This is used
  for large alignments > MI_ALIGNMENT_MAX. We use a large mimalloc
  page where the object can be aligned at an offset from the start of the segment.
  As we may need to overallocate, we need to free such pointers using `mi_free_aligned`
  to use the actual start of the memory region.
----------------------------------------------------------- */

void* _mi_os_alloc_aligned_offset(size_t size, size_t alignment, size_t offset, bool commit, bool* large, mi_stats_t* tld_stats) {
  mi_assert(offset <= MI_SEGMENT_SIZE);
  mi_assert(offset <= size);
  mi_assert((alignment % _mi_os_page_size()) == 0);
  if (offset > MI_SEGMENT_SIZE) return NULL;
  if (offset == 0) {
    // regular aligned allocation
    return _mi_os_alloc_aligned(size, alignment, commit, large, tld_stats);
  }
  else {
    // overallocate to align at an offset
    const size_t extra = _mi_align_up(offset, alignment) - offset;
    const size_t oversize = size + extra;
    void* start = _mi_os_alloc_aligned(oversize, alignment, commit, large, tld_stats);
    if (start == NULL) return NULL;
    void* p = (uint8_t*)start + extra;
    mi_assert(_mi_is_aligned((uint8_t*)p + offset, alignment));
    // decommit the overallocation at the start
    if (commit && extra > _mi_os_page_size()) {
      _mi_os_decommit(start, extra, tld_stats);
    }
    return p;
  }
}

void _mi_os_free_aligned(void* p, size_t size, size_t alignment, size_t align_offset, bool was_committed, mi_stats_t* tld_stats) {
  mi_assert(align_offset <= MI_SEGMENT_SIZE);
  const size_t extra = _mi_align_up(align_offset, alignment) - align_offset;
  void* start = (uint8_t*)p - extra;
  _mi_os_free_ex(start, size + extra, was_committed, tld_stats);
}

/* -----------------------------------------------------------
  OS memory API: reset, commit, decommit, protect, unprotect.
----------------------------------------------------------- */


// OS page align within a given area, either conservative (pages inside the area only),
// or not (straddling pages outside the area is possible)
static void* mi_os_page_align_areax(bool conservative, void* addr, size_t size, size_t* newsize) {
  mi_assert(addr != NULL && size > 0);
  if (newsize != NULL) *newsize = 0;
  if (size == 0 || addr == NULL) return NULL;

  // page align conservatively within the range
  void* start = (conservative ? mi_align_up_ptr(addr, _mi_os_page_size())
    : mi_align_down_ptr(addr, _mi_os_page_size()));
  void* end = (conservative ? mi_align_down_ptr((uint8_t*)addr + size, _mi_os_page_size())
    : mi_align_up_ptr((uint8_t*)addr + size, _mi_os_page_size()));
  ptrdiff_t diff = (uint8_t*)end - (uint8_t*)start;
  if (diff <= 0) return NULL;

  mi_assert_internal((conservative && (size_t)diff <= size) || (!conservative && (size_t)diff >= size));
  if (newsize != NULL) *newsize = (size_t)diff;
  return start;
}

static void mi_mprotect_hint(int err) {
#if defined(MI_OS_USE_MMAP) && (MI_SECURE>=2) // guard page around every mimalloc page
  if (err == ENOMEM) {
    _mi_warning_message("the previous warning may have been caused by a low memory map limit.\n"
                        "  On Linux this is controlled by the vm.max_map_count. For example:\n"
                        "  > sudo sysctl -w vm.max_map_count=262144\n");
  }
#else
  MI_UNUSED(err);
#endif
}

// Commit/Decommit memory.
// Usually commit is aligned liberal, while decommit is aligned conservative.
// (but not for the reset version where we want commit to be conservative as well)
static bool mi_os_commitx(void* addr, size_t size, bool commit, bool conservative, bool* is_zero, mi_stats_t* stats) {
  // page align in the range, commit liberally, decommit conservative
  if (is_zero != NULL) { *is_zero = false; }
  size_t csize;
  void* start = mi_os_page_align_areax(conservative, addr, size, &csize);
  if (csize == 0) return true;  // || _mi_os_is_huge_reserved(addr))
  int err = 0;
  if (commit) {
    _mi_stat_increase(&stats->committed, size);  // use size for precise commit vs. decommit
    _mi_stat_counter_increase(&stats->commit_calls, 1);
  }
  else {
    _mi_stat_decrease(&stats->committed, size);
  }

  if (commit) {
      void *const p = mmap_soft(start, csize, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (p == (void *)-1)
          err = ENOMEM;
  }
  else {
      err = munmap(start, csize);
  }
  if (err != 0) {
    _mi_warning_message("%s error: start: %p, csize: 0x%zx, err: %i\n", commit ? "commit" : "decommit", start, csize, err);
    mi_mprotect_hint(err);
  }
  mi_assert_internal(err == 0);
  return (err == 0);
}

bool _mi_os_commit(void* addr, size_t size, bool* is_zero, mi_stats_t* tld_stats) {
  MI_UNUSED(tld_stats);
  mi_stats_t* stats = &_mi_stats_main;
  return mi_os_commitx(addr, size, true, false /* liberal */, is_zero, stats);
}

bool _mi_os_decommit(void* addr, size_t size, mi_stats_t* tld_stats) {
  MI_UNUSED(tld_stats);
  mi_stats_t* stats = &_mi_stats_main;
  bool is_zero;
  return mi_os_commitx(addr, size, false, true /* conservative */, &is_zero, stats);
}

bool _mi_os_reset(void*, size_t, mi_stats_t*) {
  // 配置mimalloc选项保证此函数不会运行
  abort();
}


/* ----------------------------------------------------------------------------
Support for allocating huge OS pages (1Gib) that are reserved up-front
and possibly associated with a specific NUMA node. (use `numa_node>=0`)
-----------------------------------------------------------------------------*/


// Allocate MI_SEGMENT_SIZE aligned huge pages
void* _mi_os_alloc_huge_os_pages(size_t, int, mi_msecs_t, size_t*, size_t*) {
  // 配置mimalloc选项保证此函数不会运行
  abort();
}

// free every huge page in a range individually (as we allocated per page)
// note: needed with VirtualAlloc but could potentially be done in one go on mmap'd systems.
void _mi_os_free_huge_pages(void*, size_t, mi_stats_t*) {
  // 配置mimalloc选项保证此函数不会运行
  abort();
}

/* ----------------------------------------------------------------------------
Support NUMA aware allocation
-----------------------------------------------------------------------------*/

const _Atomic(size_t)  _mi_numa_node_count = 1; // = 0   // cache the node count

size_t _mi_os_numa_node_count_get(void) {
  // myos 目前不支持多物理CPU
  return 1;
}

int _mi_os_numa_node_get(mi_os_tld_t* tld) {
  // myos 目前不支持多物理CPU
  MI_UNUSED(tld);
  return 0;
}
