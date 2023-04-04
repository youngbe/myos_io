#!/bin/bash
BASE_CFLAGS=(-Xclang -pic-level -Xclang 0 -fno-addrsig -DNDEBUG
    -Ofast -fmerge-all-constants -ffunction-sections -fdata-sections
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-check -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all -std=gnu2x -Wall -Wextra -Wstrict-prototypes)
BASE_LD_FLAGS=(-z noexecstack -z norelro -z lazy --no-eh-frame-hdr --no-ld-generated-unwind-info --build-id=none --gc-sections)
BASE_CCLD_FLAGS=(-z noexecstack -z norelro -z lazy -Wl,--no-eh-frame-hdr -Wl,--no-ld-generated-unwind-info -Wl,--build-id=none -nolibc -nostartfiles -Wl,--gc-sections)

BLOOTLOADER_CFLAGS=(-mno-red-zone -fno-pie)
BLOOTLOADER_LD_FLAGS=(-no-pie)
BLOOTLOADER_CCLD_FLAGS=(-static) # -Wl,--print-gc-sections -Wl,--verbose -v)

mkdir -p out/startup
clang "${BASE_CFLAGS[@]}" "${BLOOTLOADER_CFLAGS[@]}" -I . -m32 -march=i386 -mtune=generic -mgeneral-regs-only startup/tty_i386.c -c -o out/startup/tty_i386.o
clang "${BASE_CFLAGS[@]}" "${BLOOTLOADER_CFLAGS[@]}" -I . -m32 -march=i386 -mtune=generic -mgeneral-regs-only startup/_start32.c -c -o out/startup/_start32.o

clang "${BASE_CFLAGS[@]}" "${BASE_CCLD_FLAGS[@]}" "${BLOOTLOADER_CFLAGS[@]}" "${BLOOTLOADER_CCLD_FLAGS[@]}" -m32 -march=x86-64-v3 \
    startup/code16.s \
    out/startup/_start32.o out/startup/tty_i386.o \
    main.c tty.c printb.c string.c e820.c qsort.c load_kernel.c enter64.c enter64.s \
    ffreestanding32.s \
    -T bootloader.ld -o bootloader.elf

objcopy -O binary \
    -j .boot_sector_text --set-section-flags .boot_sector_text=load,content,alloc \
    -j .boot_sector_data --set-section-flags .boot_sector_data=load,content,alloc \
    -j .boot_sector_magic --set-section-flags .boot_sector_magic=load,content,alloc \
    -j .text16 --set-section-flags .text16=load,content,alloc \
    -j .data16 --set-section-flags .data16=load,content,alloc \
    -j .text --set-section-flags .text=load,content,alloc \
    -j .rodata --set-section-flags .rodata=load,content,alloc \
    -j .data --set-section-flags .data=load,content,alloc \
    -j .bss --set-section-flags .bss=load,content,alloc \
    bootloader.elf bootloader.bin

dd conv=fdatasync if=bootloader.bin ibs=512 conv=sync of=boot.img
dd conv=fdatasync if=/dev/zero ibs=1M count=2 of=boot.img oflag=append conv=notrunc
qemu-img convert -f raw -O vmdk boot.img boot.vmdk
