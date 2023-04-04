#include "boot.h"

void *find_kernel_load_pos(const struct E820_Entry *const entrys, const size_t num)
{
    if (num == 1 && entrys[0].size == 0) {
        // 超级块
        if (entrys[0].type == 1 && entrys[0].extend == 1)
            return (void *)(uintptr_t)0x1000000;
        else
            bootloader_error("failed to find kernel load place!");
    }
    for (size_t i = 0; i < num; ++i) {
        size_t padsize;
        if (entrys[i].addr <= 0x1000000)
            padsize = 0x1000000 - entrys[i].addr;
        else if (entrys[i].addr <= (0x100000000 - KERNEL_SIZE_ALIGN2M)) {
            if ((entrys[i].addr & 0x1fffff) == 0)
                padsize = 0;
            else
                padsize = 0x200000 - (entrys[i].addr & 0x1fffff);
        }
        else
            bootloader_error("failed to find kernel load place!");
        if (entrys[i].size > padsize && entrys[i].size - padsize >= KERNEL_SIZE_ALIGN2M)
            return (void *)(uintptr_t)(entrys[i].addr + padsize);
    }
    bootloader_error("failed to find kernel load place!");
}

struct LDAP
{
    uint8_t size;
    uint8_t pad;
    uint16_t secs_num;
    uint16_t offset;
    uint16_t segment;
    uint64_t sec_id;
};

extern struct LDAP ldap;
extern uint32_t next_load_sec_id;

void load_kernel(void *pos)
{
    size_t secs_num_need_load = KERNEL_SIZE_ALIGN512 >> 9;
    struct biosregs ireg, oreg;
    initregs(&ireg);
    ireg.ah = 0x42;
    ireg.dl = 0x80;
    ireg.ds = (uintptr_t)&ldap >> 4;
    ireg.si = (uintptr_t)&ldap & 0xf;

    while (true) {
        if (secs_num_need_load > 127) {
            ldap.size = sizeof(struct LDAP); // sizeof(struct LDAP) == 0x10
            ldap.pad = 0;
            ldap.secs_num = 127;
            ldap.offset = 0;
            ldap.segment = (uintptr_t)_ebootloader_align16 >> 4;
            ldap.sec_id = next_load_sec_id;
            intcall(0x13, &ireg, &oreg);
            if ((oreg.eflags & 1) != 0 || oreg.ah != 0)
                bootloader_error("failed to load kernel!\n");
            memcpy(pos, _ebootloader_align16, 127 * 512);
            secs_num_need_load -= 127;
            next_load_sec_id += 127;
            pos = (void *)((uintptr_t)pos + 127 * 512);
        }
        else {
            ldap.size = sizeof(struct LDAP); // sizeof(struct LDAP) == 0x10
            ldap.pad = 0;
            ldap.secs_num = secs_num_need_load;
            ldap.offset = 0;
            ldap.segment = (uintptr_t)_ebootloader_align16 >> 4;
            ldap.sec_id = next_load_sec_id;
            intcall(0x13, &ireg, &oreg);
            if ((oreg.eflags & 1) != 0 || oreg.ah != 0)
                bootloader_error("failed to load kernel!\n");
            memcpy(pos, _ebootloader_align16, secs_num_need_load * 512);
            break;
        }
    }
}
