#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

extern alignas(512) int _ebootloader_align512;

extern struct __attribute__((packed))
{
    uint8_t size;
    uint8_t pad;
    uint16_t sec_num;
    uint16_t offset;
    uint16_t segment;
    uint64_t sec_id;
} ldap;

extern uint32_t next_load_sec_id;

void test()
{
    __asm__ volatile(""::"a"(0), "b"(0), "c"(0), "d"(0), "S"(0), "D"(0):"ebp");

    if ((uintptr_t)&_ebootloader_align512 <= 0x10000)
        return;

    if (next_load_sec_id != 0x42)
        __builtin_unreachable();

    static uintptr_t next_load_address = 0x10000;
    if (next_load_address != 0x10000)
        __builtin_unreachable();

    uintptr_t address = next_load_address;
    uint32_t sec_id;
    uintptr_t size = (uintptr_t)&_ebootloader_align512 - address;
    if (size > 0xfe00) {
        while (true) {
            sec_id = next_load_sec_id;
            next_load_sec_id += 0x7f;
            ldap.size = 0x10;
            ldap.pad = 0;
            ldap.sec_num = 0x7f;
            ldap.offset = 0;
            ldap.segment = address >> 4;
            ldap.sec_id = sec_id;
            __asm__ volatile(
                    "movw   $ldap, %%si\n\t"
                    "movb   $0x42, %%ah\n\t"
                    "movb   $0x80, %%dl\n\t"
                    "int    $0x13\n\t"
                    "jc     .Lerror\n\t"
                    "testb   %%ah, %%ah\n\t"
                    "jne    .Lerror\n\t"
                    "callw  .Lclear"
                    :"+m"(ldap)
                    :
                    :"eax", "ebx", "ecx", "edx", "ebp", "edi", "esi", "cc");
            address = next_load_address;
            address += 0xfe00;
            size = (uintptr_t)&_ebootloader_align512 - address;
            if (size <= 0xfe00)
                break;
            next_load_address = address;
        }
    }
    sec_id = next_load_sec_id;
    next_load_sec_id += size >> 9;
    ldap.size = 0x10;
    ldap.pad = 0;
    ldap.sec_num = size >> 9;
    ldap.offset = 0;
    ldap.segment = address >> 4;
    ldap.sec_id = sec_id;
    __asm__ volatile(
            "movw   $ldap, %%si\n\t"
            "movb   $0x42, %%ah\n\t"
            "movb   $0x80, %%dl\n\t"
            "int    $0x13\n\t"
            "jc     .Lerror\n\t"
            "testb   %%ah, %%ah\n\t"
            "jne    .Lerror\n\t"
            "callw  .Lclear"
            :"+m"(ldap)
            :
            :"eax", "ebx", "ecx", "edx", "ebp", "edi", "esi", "cc");
}
