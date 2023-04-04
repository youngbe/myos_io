#include <stdio.h>
#include "boot.h"

int main()
{
    struct E820_Entry entrys[512];
    entrys[0].addr = 20;
    entrys[0].size = 40;
    entrys[0].type = 1;
    entrys[0].extend = 1;
    entrys[1].addr = 70;
    entrys[1].size = 20;
    entrys[1].type = 1;
    entrys[1].extend = 1;
    entrys[2].addr = 90;
    entrys[2].size = 50;
    entrys[2].type = 1;
    entrys[2].extend = 1;
    entrys[3].addr = 100;
    entrys[3].size = 10;
    entrys[3].type = 3;
    entrys[3].extend = 1;
    entrys[4].addr = 80;
    entrys[4].size = 20;
    entrys[4].type = 1;
    entrys[4].extend = 1;
    entrys[5].addr = 80;
    entrys[5].size = 2;
    entrys[5].type = 1;
    entrys[5].extend = 1;
    entrys[6].addr = 80;
    entrys[6].size = 25;
    entrys[6].type = 1;
    entrys[6].extend = 1;
    entrys[7].addr = 300;
    entrys[7].size = 800;
    entrys[7].type = 1;
    entrys[7].extend = 1;
    entrys[8].addr = 400;
    entrys[8].size = 300;
    entrys[8].type = 1;
    entrys[8].extend = 1;
    struct E820_Entry e820_entrys[512];
    size_t e820_entrys_num = trim_e820_entrys(entrys, 9, e820_entrys);

    for (size_t i = 0; i < e820_entrys_num; ++i)
        printf("%lu::addr:%lu, size:%lu, type: %u, extend: %u\n", i, e820_entrys[i].addr, e820_entrys[i].size, e820_entrys[i].type, e820_entrys[i].extend);
}
