/* ----------------------------------------------------------------------------
Copyright (c) 2018-2021, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "mimalloc.h"
#include "mimalloc-internal.h"
#include "mimalloc-atomic.h"

#include <stdio.h>
#include <stdlib.h> // strtol
#include <string.h> // strncpy, strncat, strlen, strstr
#include <ctype.h>  // toupper
#include <stdarg.h>

#ifdef _MSC_VER
#pragma warning(disable:4996)   // strncpy, strncat
#endif


static long mi_max_error_count   = 16; // stop outputting errors after this (use < 0 for no limit)
static long mi_max_warning_count = 16; // stop outputting warnings after this (use < 0 for no limit)


int mi_version(void) mi_attr_noexcept {
  return MI_MALLOC_VERSION;
}

#ifdef _WIN32
#include <conio.h>
#endif

// --------------------------------------------------------
// Options
// These can be accessed by multiple threads and may be
// concurrently initialized, but an initializing data race
// is ok since they resolve to the same value.
// --------------------------------------------------------
typedef enum mi_init_e {
  UNINIT,       // not yet initialized
  DEFAULTED,    // not found in the environment, use default value
  INITIALIZED   // found in environment or set explicitly
} mi_init_t;

typedef struct mi_option_desc_s {
  long        value;  // the value
  mi_init_t   init;   // is it initialized yet? (from the environment)
  mi_option_t option; // for debugging: the option index should match the option
  const char* name;   // option name without `mimalloc_` prefix
  const char* legacy_name; // potential legacy v1.x option name
} mi_option_desc_t;

#define MI_OPTION(opt)                  mi_option_##opt, #opt, NULL
#define MI_OPTION_LEGACY(opt,legacy)    mi_option_##opt, #opt, #legacy

static const mi_option_desc_t options[_mi_option_last] =
{
  // stable options
  { 0, INITIALIZED, MI_OPTION(show_errors) },
  { 0, INITIALIZED, MI_OPTION(show_stats) },
  { 0, INITIALIZED, MI_OPTION(verbose) },

  // Some of the following options are experimental and not all combinations are valid. Use with care.
  { 0, INITIALIZED, MI_OPTION(eager_commit) },        // commit per segment directly (8MiB)  (but see also `eager_commit_delay`)
  { 0, INITIALIZED, MI_OPTION(deprecated_eager_region_commit) },
  { 0, INITIALIZED, MI_OPTION(deprecated_reset_decommits) },
  { 0, INITIALIZED, MI_OPTION(large_os_pages) },      // use large OS pages, use only with eager commit to prevent fragmentation of VMA's
  { 0, INITIALIZED, MI_OPTION(reserve_huge_os_pages) },  // per 1GiB huge pages
  { -1, INITIALIZED, MI_OPTION(reserve_huge_os_pages_at) }, // reserve huge pages at node N
  { 0, INITIALIZED, MI_OPTION(reserve_os_memory)     },
  { 0, INITIALIZED, MI_OPTION(deprecated_segment_cache) },  // cache N segments per thread
  { 0, INITIALIZED, MI_OPTION(page_reset) },          // reset page memory on free
  { 0, INITIALIZED, MI_OPTION_LEGACY(abandoned_page_decommit, abandoned_page_reset) },// decommit free page memory when a thread terminates  
  { 0, INITIALIZED, MI_OPTION(deprecated_segment_reset) },
  { 0, INITIALIZED, MI_OPTION(eager_commit_delay) },  // the first N segments per thread are not eagerly committed (but per page in the segment on demand)
  { 25,   INITIALIZED, MI_OPTION_LEGACY(decommit_delay, reset_delay) }, // page decommit delay in milli-seconds
  { 0,    INITIALIZED, MI_OPTION(use_numa_nodes) },    // 0 = use available numa nodes, otherwise use at most N nodes. 
  { 0,    INITIALIZED, MI_OPTION(limit_os_alloc) },    // 1 = do not use OS memory for allocation (but only reserved arenas)
  { 100,  INITIALIZED, MI_OPTION(os_tag) },            // only apple specific for now but might serve more or less related purpose
  { 16,   INITIALIZED, MI_OPTION(max_errors) },        // maximum errors that are output
  { 16,   INITIALIZED, MI_OPTION(max_warnings) },      // maximum warnings that are output
  { 8,    INITIALIZED, MI_OPTION(max_segment_reclaim)},// max. number of segment reclaims from the abandoned segments per try.  
  { 1,    INITIALIZED, MI_OPTION(allow_decommit) },    // decommit slices when no longer used (after decommit_delay milli-seconds)
  { 0,  INITIALIZED, MI_OPTION(segment_decommit_delay) }, // decommit delay in milli-seconds for freed segments
  { 0,    INITIALIZED, MI_OPTION(decommit_extend_delay) },
  { 0,    INITIALIZED, MI_OPTION(destroy_on_exit)}     // release all OS memory on process exit; careful with dangling pointer or after-exit frees!
};

void _mi_options_init(void) {
  // called on process load; should not be called before the CRT is initialized!
  // (e.g. do not call this from process_init as that may run before CRT initialization)
  //mi_add_stderr_output(); // now it safe to use stderr for output
  for(int i = 0; i < _mi_option_last; i++ ) {
    mi_option_t option = (mi_option_t)i;
    long l = mi_option_get(option); MI_UNUSED(l); // initialize
    // if (option != mi_option_verbose)
    {
      mi_option_desc_t* desc = &options[option];
      _mi_verbose_message("option '%s': %ld\n", desc->name, desc->value);
    }
  }
  mi_max_error_count = mi_option_get(mi_option_max_errors);
  mi_max_warning_count = mi_option_get(mi_option_max_warnings);
}

mi_decl_nodiscard long mi_option_get(mi_option_t option) {
  mi_assert(option >= 0 && option < _mi_option_last);
  if (option < 0 || option >= _mi_option_last) return 0;
  mi_option_desc_t* desc = &options[option];
  mi_assert(desc->option == option);  // index should match the option
  if mi_unlikely(desc->init == UNINIT) {
    abort();
  }
  return desc->value;
}

mi_decl_nodiscard long mi_option_get_clamp(mi_option_t option, long min, long max) {
  long x = mi_option_get(option);
  return (x < min ? min : (x > max ? max : x));
}

mi_decl_nodiscard bool mi_option_is_enabled(mi_option_t option) {
  return (mi_option_get(option) != 0);
}

// --------------------------------------------------------
// Messages, all end up calling `_mi_fputs`.
// --------------------------------------------------------

void _mi_fputs(mi_output_fun*, void*, const char*, const char*) {
  abort();
}

void _mi_verbose_message(const char*, ...) {
  if (!mi_option_is_enabled(mi_option_verbose)) return;
  abort();
}

void _mi_warning_message(const char*, ...) {
  if (!mi_option_is_enabled(mi_option_verbose)) {
    if (!mi_option_is_enabled(mi_option_show_errors)) return;
    abort();
  }
  abort();
}

void _mi_error_message(int, const char*, ...) {
  abort();
}
