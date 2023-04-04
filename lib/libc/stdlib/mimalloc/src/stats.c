/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc-internal.h"
#include "mimalloc-atomic.h"

#include <stdio.h>  // fputs, stderr
#include <string.h> // memset

#if defined(_MSC_VER) && (_MSC_VER < 1920)
#pragma warning(disable:4204)  // non-constant aggregate initializer
#endif

/* -----------------------------------------------------------
  Statistics operations
----------------------------------------------------------- */

static bool mi_is_in_main(void* stat) {
  return ((uint8_t*)stat >= (uint8_t*)&_mi_stats_main
         && (uint8_t*)stat < ((uint8_t*)&_mi_stats_main + sizeof(mi_stats_t)));
}

static void mi_stat_update(mi_stat_count_t* stat, int64_t amount) {
  if (amount == 0) return;
  if (mi_is_in_main(stat))
  {
    // add atomically (for abandoned pages)
    int64_t current = mi_atomic_addi64_relaxed(&stat->current, amount);
    mi_atomic_maxi64_relaxed(&stat->peak, current + amount);
    if (amount > 0) {
      mi_atomic_addi64_relaxed(&stat->allocated,amount);
    }
    else {
      mi_atomic_addi64_relaxed(&stat->freed, -amount);
    }
  }
  else {
    // add thread local
    stat->current += amount;
    if (stat->current > stat->peak) stat->peak = stat->current;
    if (amount > 0) {
      stat->allocated += amount;
    }
    else {
      stat->freed += -amount;
    }
  }
}

void _mi_stat_counter_increase(mi_stat_counter_t* stat, size_t amount) {
  if (mi_is_in_main(stat)) {
    mi_atomic_addi64_relaxed( &stat->count, 1 );
    mi_atomic_addi64_relaxed( &stat->total, (int64_t)amount );
  }
  else {
    stat->count++;
    stat->total += amount;
  }
}

void _mi_stat_increase(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, (int64_t)amount);
}

void _mi_stat_decrease(mi_stat_count_t* stat, size_t amount) {
  mi_stat_update(stat, -((int64_t)amount));
}

// must be thread safe as it is called from stats_merge
static void mi_stat_add(mi_stat_count_t* stat, const mi_stat_count_t* src, int64_t unit) {
  if (stat==src) return;
  if (src->allocated==0 && src->freed==0) return;
  mi_atomic_addi64_relaxed( &stat->allocated, src->allocated * unit);
  mi_atomic_addi64_relaxed( &stat->current, src->current * unit);
  mi_atomic_addi64_relaxed( &stat->freed, src->freed * unit);
  // peak scores do not work across threads..
  mi_atomic_addi64_relaxed( &stat->peak, src->peak * unit);
}

static void mi_stat_counter_add(mi_stat_counter_t* stat, const mi_stat_counter_t* src, int64_t unit) {
  if (stat==src) return;
  mi_atomic_addi64_relaxed( &stat->total, src->total * unit);
  mi_atomic_addi64_relaxed( &stat->count, src->count * unit);
}

// must be thread safe as it is called from stats_merge
static void mi_stats_add(mi_stats_t* stats, const mi_stats_t* src) {
  if (stats==src) return;
  mi_stat_add(&stats->segments, &src->segments,1);
  mi_stat_add(&stats->pages, &src->pages,1);
  mi_stat_add(&stats->reserved, &src->reserved, 1);
  mi_stat_add(&stats->committed, &src->committed, 1);
  mi_stat_add(&stats->reset, &src->reset, 1);
  mi_stat_add(&stats->page_committed, &src->page_committed, 1);

  mi_stat_add(&stats->pages_abandoned, &src->pages_abandoned, 1);
  mi_stat_add(&stats->segments_abandoned, &src->segments_abandoned, 1);
  mi_stat_add(&stats->threads, &src->threads, 1);

  mi_stat_add(&stats->malloc, &src->malloc, 1);
  mi_stat_add(&stats->segments_cache, &src->segments_cache, 1);
  mi_stat_add(&stats->normal, &src->normal, 1);
  mi_stat_add(&stats->huge, &src->huge, 1);
  mi_stat_add(&stats->large, &src->large, 1);

  mi_stat_counter_add(&stats->pages_extended, &src->pages_extended, 1);
  mi_stat_counter_add(&stats->mmap_calls, &src->mmap_calls, 1);
  mi_stat_counter_add(&stats->commit_calls, &src->commit_calls, 1);

  mi_stat_counter_add(&stats->page_no_retire, &src->page_no_retire, 1);
  mi_stat_counter_add(&stats->searches, &src->searches, 1);
  mi_stat_counter_add(&stats->normal_count, &src->normal_count, 1);
  mi_stat_counter_add(&stats->huge_count, &src->huge_count, 1);
  mi_stat_counter_add(&stats->large_count, &src->large_count, 1);
#if MI_STAT>1
  for (size_t i = 0; i <= MI_BIN_HUGE; i++) {
    if (src->normal_bins[i].allocated > 0 || src->normal_bins[i].freed > 0) {
      mi_stat_add(&stats->normal_bins[i], &src->normal_bins[i], 1);
    }
  }
#endif
}

#if MI_STAT>1
static void mi_stats_print_bins(const mi_stat_count_t* bins, size_t max, const char* fmt, mi_output_fun* out, void* arg) {
  bool found = false;
  char buf[64];
  for (size_t i = 0; i <= max; i++) {
    if (bins[i].allocated > 0) {
      found = true;
      int64_t unit = _mi_bin_size((uint8_t)i);
      snprintf(buf, 64, "%s %3lu", fmt, (long)i);
      mi_stat_print(&bins[i], buf, unit, out, arg);
    }
  }
  if (found) {
    _mi_fprintf(out, arg, "\n");
    mi_print_header(out, arg);
  }
}
#endif

static mi_stats_t* mi_stats_get_default(void) {
  mi_heap_t* heap = mi_heap_get_default();
  return &heap->tld->stats;
}

static void mi_stats_merge_from(mi_stats_t* stats) {
  if (stats != &_mi_stats_main) {
    mi_stats_add(&_mi_stats_main, stats);
    memset(stats, 0, sizeof(mi_stats_t));
  }
}

void mi_stats_reset(void) mi_attr_noexcept {
  mi_stats_t* stats = mi_stats_get_default();
  if (stats != &_mi_stats_main) { memset(stats, 0, sizeof(mi_stats_t)); }
  memset(&_mi_stats_main, 0, sizeof(mi_stats_t));
  //if (mi_process_start == 0) { mi_process_start = _mi_clock_start(); };
}

void mi_stats_merge(void) mi_attr_noexcept {
  mi_stats_merge_from( mi_stats_get_default() );
}

void _mi_stats_done(mi_stats_t* stats) {  // called from `mi_thread_done`
  mi_stats_merge_from(stats);
}


void mi_stats_print(void*) mi_attr_noexcept {
  abort();
}

// ----------------------------------------------------------------
// Basic timer for convenience; use milli-seconds to avoid doubles
// ----------------------------------------------------------------
mi_msecs_t _mi_clock_now(void) {
  abort();
}

// --------------------------------------------------------
// Basic process statistics
// --------------------------------------------------------

mi_decl_export void mi_process_info(size_t*, size_t*, size_t*, size_t*, size_t*, size_t*, size_t*, size_t*) mi_attr_noexcept
{
  abort();
}
