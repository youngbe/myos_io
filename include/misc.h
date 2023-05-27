#pragma once

#include <io.h>

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define container_of(ptr, type, member) (type *)((char *)(ptr) - offsetof(type, member))
