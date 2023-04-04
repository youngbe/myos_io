#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <string.h>

int bcmp(const void *s1, const void *s2, size_t n)
{
	return memcmp(s1, s2, n);
}
