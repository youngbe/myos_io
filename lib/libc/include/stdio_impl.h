#ifndef _STDIO_IMPL_H
#define _STDIO_IMPL_H

#include <stdio.h>
#undef stdin
#undef stdout
#undef stderr
extern FILE __stdin_FILE;
extern FILE __stdout_FILE;
extern FILE __stderr_FILE;
#define stdin (&__stdin_FILE)
#define stdout (&__stdout_FILE)
#define stderr (&__stderr_FILE)

#include <stdbool.h>
#include <threads.h>

#define UNGET 8

#define FLOCK(f) const bool __need_unlock = ((f)->need_lock && (__lockfile((f)), true))
#define FUNLOCK(f) do { if (__need_unlock) __unlockfile((f)); } while (0)

// 文件不可被关闭
#define F_PERM 1
// 文件不可读
#define F_NORD 4
// 文件不可写
#define F_NOWR 8
#define F_EOF 16
#define F_ERR 32
#define F_SVB 64
// append flag
#define F_APP 128

struct _IO_FILE {
	unsigned flags;
	unsigned char *rpos, *rend;
	unsigned char *wend, *wpos;
	unsigned char *wbase;
	size_t (*write)(FILE *, const unsigned char *, size_t);
	unsigned char *buf;
	size_t buf_size;
	int mode;
    bool need_lock;
	mtx_t mutex;
	int lbf;
	void *cookie;
};


int __lockfile(FILE *);
void __unlockfile(FILE *);

int __toread(FILE *);
int __towrite(FILE *);

int __overflow(FILE *, int), __uflow(FILE *);

size_t __fwritex(const unsigned char *, size_t, FILE *);

#define feof(f) ((f)->flags & F_EOF)
#define ferror(f) ((f)->flags & F_ERR)

#define getc_unlocked(f) \
	( ((f)->rpos != (f)->rend) ? *(f)->rpos++ : __uflow((f)) )

#define putc_unlocked(c, f) \
	( (((unsigned char)(c)!=(f)->lbf && (f)->wpos!=(f)->wend)) \
	? *(f)->wpos++ = (unsigned char)(c) \
	: __overflow((f),(unsigned char)(c)) )

#endif
