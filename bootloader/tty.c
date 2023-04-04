// 这个文件引用自Linux源代码：https://elixir.bootlin.com/linux/latest/source/arch/x86/boot/compressed/misc.c
// 并作了一些修改

#include "boot.h"

#define CHAR_ATTR 0b00001111
#define ROWS 25
#define COLS 80

#define VIDPORT 0x3d4
#define VIDMEM ((char *)0xb8000)

int x, y;

void console_init()
{
    // 获取当前光标位置
    size_t pos;
    outb(0x0F, 0x3D4);
    pos = inb(0x3D5);
    outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;

    if (pos >= COLS * ROWS)
        pos = 0;
    else
        pos <<= 1;

    // 清空屏幕
    for (size_t i = 0; i < pos; i += 2)
        VIDMEM[i + 1] = CHAR_ATTR;
    for (size_t i = pos; i < ROWS * COLS; i += 2) {
        VIDMEM[i] = '\0';
        VIDMEM[i + 1] = CHAR_ATTR;
    }

    __asm__ volatile (""::"m"(*(char (*)[ROWS * COLS * 2])VIDMEM):);

    x = (pos >> 1) % 80;
    y = (pos >> 1) / 80;
}

static void scroll(void)
{
	int i;

	memmove(VIDMEM, VIDMEM + COLS * 2, (ROWS - 1) * COLS * 2);
	for (i = (ROWS - 1) * COLS * 2; i < ROWS * COLS * 2; i += 2)
		VIDMEM[i] = '\0';
}

void __putstr(const char *s)
{
	int pos;
	char c;

	while ((c = *s++) != '\0') {
		if (c == '\n') {
			x = 0;
			if (++y >= ROWS) {
				scroll();
				y--;
			}
		} else {
			VIDMEM[(x + COLS * y) * 2] = c;
			if (++x >= COLS) {
				x = 0;
				if (++y >= ROWS) {
					scroll();
					y--;
				}
			}
		}
	}

	pos = (x + COLS * y) * 2;	/* Update cursor position */
	outb(14, VIDPORT);
	outb(0xff & (pos >> 9), VIDPORT+1);
	outb(15, VIDPORT);
	outb(0xff & (pos >> 1), VIDPORT+1);
}
