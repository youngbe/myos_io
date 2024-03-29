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

#include <fd.h>
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
	size_t (*read)(FILE *, unsigned char *, size_t);
	size_t (*write)(FILE *, const unsigned char *, size_t);
	off_t (*seek)(FILE *, off_t, int);
	unsigned char *buf;
	size_t buf_size;
	FILE *prev, *next;
    struct FD *fd;
	int mode;
    bool need_lock;
	mtx_t mutex;
	int lbf;
	void *cookie;
	unsigned char *shend;
	off_t shlim, shcnt;
};

extern FILE *volatile __stdin_used;
extern FILE *volatile __stdout_used;
extern FILE *volatile __stderr_used;

int __lockfile(FILE *);
void __unlockfile(FILE *);

size_t __stdio_read(FILE *, unsigned char *, size_t);
size_t __stdio_write(FILE *, const unsigned char *, size_t);
off_t __stdio_seek(FILE *, off_t, int);
int __toread(FILE *);
int __towrite(FILE *);

int __overflow(FILE *, int), __uflow(FILE *);

size_t __fwritex(const unsigned char *, size_t, FILE *);
FILE *__ofl_add(FILE *f);
FILE **__ofl_lock(void);
void __ofl_unlock(void);

#define feof(f) ((f)->flags & F_EOF)
#define ferror(f) ((f)->flags & F_ERR)

#define getc_unlocked(f) \
	( ((f)->rpos != (f)->rend) ? *(f)->rpos++ : __uflow((f)) )

#define putc_unlocked(c, f) \
	( (((unsigned char)(c)!=(f)->lbf && (f)->wpos!=(f)->wend)) \
	? *(f)->wpos++ = (unsigned char)(c) \
	: __overflow((f),(unsigned char)(c)) )

#endif
