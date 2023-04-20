#pragma once
#include <stddef.h>

struct FD;

size_t write(struct FD *fd, const void *buf, size_t size);
size_t read(struct FD *fd, void *buf, size_t size);
