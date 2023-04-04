#pragma once

struct mi_heap_s;
typedef struct mi_heap_s mi_heap_t;
void set_current_heap(mi_heap_t* heap);
mi_heap_t* get_current_heap(void);
