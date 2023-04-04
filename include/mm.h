#pragma once
#include "types.h"

#include <stddef.h>


#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */
#define PROT_EXEC	0x4		/* page can be executed */
#define PROT_NONE	0x0		/* page can not be accessed */

#define MAP_ANONYMOUS	0x20		/* don't use a file */
#define MAP_PRIVATE	0x02		/* Changes are private */

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void *mmap_soft(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#define MAP_NO_MAPPED 0
#define MAP_ALL_MAPPED 1
#define MAP_PART_MAPPED 2
int check_map(const void *addr, size_t length);

size_t estimate_free_mem(void);
