#include "io.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void config_x2apic(void)
{
    // 启用x2apic
    // 读取 && 配置 ia32_apic_base_msr
    uint64_t temp = rdmsr_volatile_seq(0x1b);
    if ((temp & (1 << 11)) == 0) {
        puts("local APIC is disabled!");
        abort();
    }
    if ((temp & (1 << 10)) == 0) {
        temp |= (1 << 10);
        wrmsr_volatile_seq(0x1b, temp);
    }
    
    // 配置虚拟中断到中断号0xff
    // 读取 && 配置 Spurious Interrupt Vector Register
    temp = rdmsr_volatile_seq(0x80F);
    // 设置第8位以启用local apic
    // 设置虚拟中断号为0xff
    temp |= (1 << 8) + 0xff;
    // 清空第12位：关闭 抑制EOI Broadcasts
    // 清空第9位：关闭 Focus Processor Checking
    temp &= ~(uint64_t)((1 << 9) | (1 << 12));
    wrmsr_volatile_seq(0x80f, temp);

    // 读取 && 配置 Divide Configuration Register
    temp = rdmsr_volatile_seq(0x83e);
    // 设置 最大除数 128 ，可能性能更快一些？
    // 清空bit 0 2, 设置 bit 1 3
    temp &= ~(uint64_t)(0b1 | 0b100);
    temp |= 0b10 | 0b1000;
    wrmsr_volatile_seq(0x83e, temp);

    // 读取 && 配置 LVT Timer register
    temp = rdmsr_volatile_seq(0x832);
    // 设置 Time Mode == Periodic : 周期性发送时钟中断
    // Mask == not Masked ：不要屏蔽
    // Delivery Status : 读取位，不管它
    // Vector == 32 : 中断向量号
    temp &= ~(uint64_t)((1 << 16) | (0b11 << 17) | 0xff);
    temp |= 32 | 0b01 << 17;
    wrmsr_volatile_seq(0x832, temp);

    // 设置count
    wrmsr_volatile_seq(0x838, 491520);
}
