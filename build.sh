#!/bin/bash
LTO_CFLAGS=(-flto=full -fno-split-lto-unit -fwhole-program-vtables)
LTO_CCLDFLAGS=(-Wl,--lto-partitions=1 -Wl,--lto-whole-program-visibility -Wl,--no-lto-legacy-pass-manager)

BASE_CPPFLAGS=(-std=gnu2x -DNDEBUG -D__NO_CTYPE -U_FORTIFY_SOURCE -U__linux__ -U__linux -Ulinux -U__gnu_linux__)
BASE_CFLAGS=(-Xclang -pic-level -Xclang 0 -fno-addrsig -march=x86-64-v3 -mtune=generic
    -Ofast -fmerge-all-constants -ffunction-sections -fdata-sections
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-check -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all
    -mno-red-zone -std=gnu2x -Wall -Wextra -Wstrict-prototypes -Wshadow)
BASE_LDFLAGS=(-z noexecstack -z norelro -z lazy --no-eh-frame-hdr --build-id=none --gc-sections)
BASE_CCLDFLAGS=(-fuse-ld=lld -z noexecstack -z norelro -z lazy -Wl,--no-eh-frame-hdr -Wl,--build-id=none -nolibc -nostartfiles -Wl,--gc-sections)
#BASE_CCLDFLAGS=(-Wl,--no-ld-generated-unwind-info -z noexecstack -z norelro -z lazy -Wl,--no-eh-frame-hdr -Wl,--build-id=none -nolibc -nostartfiles -Wl,--gc-sections "${LTO_LDFLAGS[@]}")

BOOTLOADER_CPPFLAGS=(-I bootloader)
BOOTLOADER_CFLAGS=(-fno-pie -m32)
BOOTLOADER_LDFLAGS=(-no-pie)
BOOTLOADER_CCLDFLAGS=(-static -m32)

KERNEL_CPPFLAGS=(-I include)
KERNEL_CFLAGS=(-fpie)
KERNEL_LDFLAGS=(-pie)
KERNEL_CCLDFLAGS=(-static-pie)

hang_run()
{
    while [[ $(jobs -r | wc -l) -ge $(nproc) ]]
    do
        wait -n
    done
    "$@" &
}

# 编译libc的math和complex模块，禁用-fast-math，启用-frounding-math
# libc:math complex
src_files=(lib/libc/math/*.c lib/libc/complex/*.c)
out_files=()
for src_file in "${src_files[@]}"
do
    out_file="out/${src_file}.o"
    mkdir -p $(dirname ${out_file})
    hang_run clang -I lib/libc/include "${BASE_CPPFLAGS[@]}" "${KERNEL_CPPFLAGS[@]}" \
        "${BASE_CFLAGS[@]}" "${KERNEL_CFLAGS[@]}" -ffp-contract=on -fno-fast-math -frounding-math -Wno-unused-but-set-variable -Wno-unused-parameter \
        "$src_file" -c -o "$out_file"
    out_files+=($out_file)
done
out_files+=(lib/libc/math/*.s)

# libc: string stdio stdlib wchar errno ctypes
out_files+=(lib/libc/string/*.s)
src_files=(lib/libc/string/*.c lib/libc/stdio/*.c lib/libc/stdlib/*.c lib/libc/wchar/*.c lib/libc/errno/*.c lib/libc/ctype/*.c)
for src_file in "${src_files[@]}"
do
    out_file="out/${src_file}.o"
    mkdir -p $(dirname ${out_file})
    hang_run clang -I lib/libc/include "${BASE_CPPFLAGS[@]}" "${KERNEL_CPPFLAGS[@]}" -D_GNU_SOURCE \
        "${BASE_CFLAGS[@]}" "${KERNEL_CFLAGS[@]}" -Wno-sign-compare -Wno-shift-op-parentheses -Wno-shadow -Wno-constant-logical-operand -Wno-unused-parameter \
        "$src_file" -c -o "$out_file"
    out_files+=($out_file)
done

# mimalloc
mkdir -p out/lib/libc/stdlib/mimalloc/src
hang_run clang -I lib/libc/stdlib/mimalloc/include "${BASE_CPPFLAGS[@]}" "${KERNEL_CPPFLAGS[@]}" \
    "${BASE_CFLAGS[@]}" "${KERNEL_CFLAGS[@]}" -Wno-static-in-inline -Wno-constant-logical-operand -Wno-unused-but-set-variable -Wno-incompatible-pointer-types-discards-qualifiers -Wno-deprecated-pragma \
    lib/libc/stdlib/mimalloc/src/static.c -c -o out/lib/libc/stdlib/mimalloc/src/static.o
out_files+=(out/lib/libc/stdlib/mimalloc/src/static.o)

# sched模块
out_files+=(sched/mutex/mtx_lock.s sched/mutex/find_hook.s sched/timer_isr/timer_isr.s
sched/empty_loop.s sched/spurious_isr.s sched/empty_isr.s sched/thread_start.s sched/abort_handler.s sched/new_thread_isr.s)
src_files=(sched/static.c)
for src_file in "${src_files[@]}"
do
    out_file="out/${src_file}.i"
    mkdir -p $(dirname ${out_file})
    hang_run clang -I sched/include "${BASE_CPPFLAGS[@]}" "${KERNEL_CPPFLAGS[@]}" \
        "$src_file" -E -o "$out_file"
    out_files+=($out_file)
done

wait

clang "${BASE_CPPFLAGS[@]}" "${KERNEL_CPPFLAGS[@]}" \
    "${BASE_CFLAGS[@]}" "${KERNEL_CFLAGS[@]}" "${LTO_CFLAGS[@]}" \
    "${BASE_CCLDFLAGS[@]}" "${KERNEL_CCLDFLAGS[@]}" "${LTO_CCLDFLAGS[@]}" \
    start/_start.s start/pie_relocate.c start/ap_start16.s start/ap_start64.s \
    driver/tty.c driver/config_x2apic.c mm/static.c \
    main.c \
    "${out_files[@]}" \
    -T kernel.ld -o kernel.elf

objcopy -O binary \
    -j .text --set-section-flags .text=load,content,alloc \
    -j .rodata --set-section-flags .rodata=load,content,alloc \
    -j .data.rel.ro --set-section-flags .data.rel.ro=load,content,alloc \
    -j .data --set-section-flags .data=load,content,alloc \
    -j .bss --set-section-flags .bss=load,content,alloc \
    -j .preinit_array --set-section-flags .preinit_array=load,content,alloc \
    -j .init_array --set-section-flags .init_array=load,content,alloc \
    -j .fini_array --set-section-flags .fini_array=load,content,alloc \
    -j .rela.dyn --set-section-flags .rela.dyn=load,content,alloc \
    kernel.elf kernel.bin

kernel_size=$(stat -c '%s' kernel.bin)

clang "${BASE_CPPFLAGS[@]}" "${BOOTLOADER_CPPFLAGS[@]}" -DKERNEL_SIZE=${kernel_size} \
    "${BASE_CFLAGS[@]}" "${BOOTLOADER_CFLAGS[@]}" \
    "${BASE_CCLDFLAGS[@]}" "${BOOTLOADER_CCLDFLAGS[@]}" \
    bootloader/code16.s bootloader/main.c bootloader/tty.c bootloader/printb.c bootloader/string.c bootloader/e820.c bootloader/qsort.c bootloader/load_kernel.c bootloader/page_table.c bootloader/enter64.s bootloader/disable_8259a.c \
    bootloader/ffreestanding32.s \
    -T bootloader/bootloader.ld -o bootloader.elf

objcopy -O binary \
    -j .boot_sector_text --set-section-flags .boot_sector_text=load,content,alloc \
    -j .boot_sector_data --set-section-flags .boot_sector_data=load,content,alloc \
    -j .boot_sector_magic --set-section-flags .boot_sector_magic=load,content,alloc \
    -j .text16 --set-section-flags .text16=load,content,alloc \
    -j .data16 --set-section-flags .data16=load,content,alloc \
    -j .text --set-section-flags .text=load,content,alloc \
    -j .rodata --set-section-flags .rodata=load,content,alloc \
    -j .data.rel.ro --set-section-flags .data.rel.ro=load,content,alloc \
    -j .data --set-section-flags .data=load,content,alloc \
    -j .bss --set-section-flags .bss=load,content,alloc \
    bootloader.elf bootloader.bin

dd conv=fdatasync if=bootloader.bin ibs=512 conv=sync of=boot.img
dd conv=fdatasync if=kernel.bin ibs=512 conv=sync of=boot.img oflag=append conv=notrunc
dd conv=fdatasync if=/dev/zero ibs=1M count=2 of=boot.img oflag=append conv=notrunc
qemu-img convert -f raw -O vmdk boot.img boot.vmdk
