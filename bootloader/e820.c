#include "boot.h"

size_t detect_memory_e820(struct E820_Entry *const e820_entrys, const size_t limit)
{
    printb("Getting memory map (E820)\n");
    size_t e820_entrys_num = 0;
    struct biosregs iregs, oregs;
    initregs(&iregs);
    iregs.ax = 0xe820; // eax == 0xe820
    // iregs.ebx = 0;
    iregs.cl = sizeof(struct E820_Entry); // sizeof(struct E820_Entry) is 24; ecx == 24
    iregs.edx = 0x534D4150;
    iregs.es = (uintptr_t)e820_entrys >> 4;
    iregs.dil = (uintptr_t)e820_entrys & 0xf; // es:di = e820_entrys
    while (true) {
        intcall(0x15, &iregs, &oregs);
        if ((oregs.eflags & 1) != 0)
            break;
        if (oregs.eax != 0x534D4150)
            goto error;

        if (oregs.cl == 20) {
            if (e820_entrys[e820_entrys_num].size == 0)
                goto skip_entry;
            printb("get a 20 bytes entry: addr: %llu, size: %llu, type: %u\n", e820_entrys[e820_entrys_num].addr, e820_entrys[e820_entrys_num].size, e820_entrys[e820_entrys_num].type);
            e820_entrys[e820_entrys_num].extend = 1;
        }
        else if (oregs.cl == 24 ) {
            if ((e820_entrys[e820_entrys_num].extend & 1) == 0 || e820_entrys[e820_entrys_num].size == 0)
                goto skip_entry;
            printb("get a 24 bytes entry: addr: %llu, size: %llu, type: %u, extend: %u\n", e820_entrys[e820_entrys_num].addr, e820_entrys[e820_entrys_num].size, e820_entrys[e820_entrys_num].type, e820_entrys[e820_entrys_num].extend);
        }
        else
            goto error;

        ++e820_entrys_num;
skip_entry:
        if (oregs.ebx == 0)
            break;
        if (e820_entrys_num >= limit)
            goto error;
        iregs.ebx = oregs.ebx;
        iregs.di += sizeof(struct E820_Entry);
    }
    if (e820_entrys_num == 0)
        goto error;

    return e820_entrys_num;

error:
    bootloader_error("error in running E820!\n");
}

struct Block
{
    uint64_t addr;
    uint32_t type;
    uint32_t extend;
    bool exist;
};

static int blocks_cmp(const void *a, const void *b)
{
    if (((const struct Block *)a)->addr > ((const struct Block *)b)->addr)
        return 1;
    if (((const struct Block *)a)->addr < ((const struct Block *)b)->addr)
        return -1;
    return 0;
}

size_t trim_e820_entrys(const struct E820_Entry *const in_entrys, const size_t in_entrys_num, struct E820_Entry *const out_entrys)
{
    struct Block blocks[in_entrys_num << 1];
    size_t blocks_num = 0;
    for (size_t i = 0; i < in_entrys_num; ++i) {
        if (in_entrys[i].addr > PHY_ADDR_MAX)
            continue;
        blocks[blocks_num++].addr = in_entrys[i].addr;
        if (PHY_ADDR_MAX - in_entrys[i].addr < in_entrys[i].size)
            continue;
        blocks[blocks_num++].addr = in_entrys[i].addr + in_entrys[i].size;
    }
    if (blocks_num == 0)
        return 0;
    qsort(blocks, blocks_num, sizeof(*blocks), blocks_cmp);
    size_t i = 0;
    goto label_in;
    for (; i < blocks_num; ++i) {
        if (blocks[i].addr == blocks[i - 1].addr)
            continue;
label_in:;
        size_t ii;
        for (ii = 0; ii < in_entrys_num; ++ii) {
            if (blocks[i].addr >= in_entrys[ii].addr && blocks[i].addr - in_entrys[ii].addr < in_entrys[ii].size) {
                blocks[i].exist = true;
                blocks[i].type = in_entrys[ii].type;
                blocks[i].extend = in_entrys[ii].extend;
                goto label_find1;
            }
        }
        // 一个都没找到
        blocks[i].exist = false;
        continue;
label_find1:
        for (++ii; ii < in_entrys_num; ++ii) {
            if (blocks[i].addr >= in_entrys[ii].addr && blocks[i].addr - in_entrys[ii].addr < in_entrys[ii].size) {
                if (blocks[i].type != in_entrys[ii].type || blocks[i].extend != in_entrys[ii].extend) {
                    // 重叠且属性不同
                    blocks[i].type = 2;
                    blocks[i].extend = 1;
                    break;
                }
            }
        }
    }

    size_t out_entrys_num;
    size_t next_i;
#if PHY_ADDR_MAX == UINT64_MAX
    // 判断是不是只有一个块的特殊情况
    // 第一个块肯定存在
    if (blocks[0].addr != 0) {
        // 不是，进入循环
        i = 0;
        out_entrys_num = 0;
        goto label_in2;
    }
    else {
        out_entrys[0].addr = 0;
        out_entrys[0].type = blocks[0].type;
        out_entrys[0].extend = blocks[0].extend;
        next_i = 1;
        i = 0;
        while (true) {
            if (next_i >= blocks_num) {
                // 超级块出现
                out_entrys[0].size = 0;
                return 1;
            }
            if (blocks[next_i].addr == blocks[i].addr)
                ++next_i;
            else {
                // 找到了下一个，先检查一下属性是否是相同的
                if (blocks[next_i].exist == blocks[i].exist && blocks[next_i].type == blocks[i].type && blocks[next_i].extend == blocks[i].extend) {
                    // 属性是相同的，略过这个条目
                    i = next_i;
                    ++next_i;
                    continue;
                }
                // 不是，进入循环
                out_entrys_num = 0;
                goto label_in3;
            }
        }
    }
#endif

    out_entrys_num = 0;
    i = 0;
    goto label_in2;
    while (true) {
        if (blocks[i].exist == false) {
            next_i = i + 1;
            while (true) {
                if (next_i >= blocks_num) {
                    // 没有下一个
                    goto label_out;
                }
                if (blocks[next_i].addr == blocks[i].addr)
                    ++next_i;
                else {
                    i = next_i;
                    // 找到了下一个，下一个一定是exist的
                    goto label_in2;
                }
            }
        }
        else {
label_in2:
            out_entrys[out_entrys_num].addr = blocks[i].addr;
            out_entrys[out_entrys_num].type = blocks[i].type;
            out_entrys[out_entrys_num].extend = blocks[i].extend;
            next_i = i + 1;
            while (true) {
                if (next_i >= blocks_num) {
                    // 没有下一个
                    out_entrys[out_entrys_num].size = PHY_ADDR_MAX - out_entrys[out_entrys_num].addr + 1;
                    ++out_entrys_num;
                    goto label_out;
                }
                if (blocks[next_i].addr == blocks[i].addr)
                    ++next_i;
                else {
                    // 找到了下一个，先检查一下属性是否是相同的
                    if (blocks[next_i].exist == blocks[i].exist && blocks[next_i].type == blocks[i].type && blocks[next_i].extend == blocks[i].extend) {
                        // 属性是相同的，略过这个条目
                        i = next_i;
                        ++next_i;
                        continue;
                    }
label_in3:
                    out_entrys[out_entrys_num].size = blocks[next_i].addr - out_entrys[out_entrys_num].addr;
                    ++out_entrys_num;
                    i = next_i;
                    break;
                }
            }
        }
    }
label_out:
    return out_entrys_num;
}
