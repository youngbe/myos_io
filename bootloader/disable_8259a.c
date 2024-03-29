#include "boot.h"

void disable_8259a(void)
{
    outb(0x11, 0x20);
    outb(0, 0x80); // io_wait
    
    outb(0x11, 0xa0);
    outb(0, 0x80);

    outb(0xf8, 0x21);
    outb(0, 0x80);

    outb(0xf8, 0xa1);
    outb(0, 0x80);

    outb(0x4, 0x21);
    outb(0, 0x80);

    outb(0x2, 0xa1);
    outb(0, 0x80);

    outb(0x1, 0x21);
    outb(0, 0x80);

    outb(0x1, 0xa1);
    outb(0, 0x80);

    // 这句话要在前面，如果先屏蔽了前面的芯片，后面的芯片接收不到屏蔽信号
    outb(0xff, 0xa1);
    outb(0xff, 0x21);
}
