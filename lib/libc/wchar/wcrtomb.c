#include <stdlib.h>
#include <wchar.h>
#include <errno.h>

#define IS_CODEUNIT(c) ((unsigned)(c)-0xdf80 < 0x80)

size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict st)
{
	if (!s) return 1;
	if ((unsigned)wc < 0x80) {
		*s = wc;
		return 1;
	} else {
		if (!IS_CODEUNIT(wc)) {
			errno = EILSEQ;
			return -1;
		}
		*s = wc;
		return 1;
	}
}
