#!/bin/sh
# LuaMetal ISO builder — runs inside a Linux container with xorriso + mtools.
# Produces /build/LuaMetal.iso (BIOS + UEFI El Torito hybrid).
set -e

ROOT=/src
BUILD=/src/build
ISO_ROOT="$BUILD/iso_root"
OUT="$BUILD/LuaMetal.iso"

rm -rf "$ISO_ROOT"
mkdir -p "$ISO_ROOT/EFI/BOOT"

# ESP contents
cp "$BUILD/esp/kernel.elf"      "$ISO_ROOT/"
cp "$BUILD/esp/limine.conf"     "$ISO_ROOT/"
cp "$BUILD/esp/init.lua"        "$ISO_ROOT/"
cp "$BUILD/esp/EFI/BOOT/BOOTX64.EFI" "$ISO_ROOT/EFI/BOOT/"

# Limine BIOS/UEFI CD boot files
cp "$ROOT/limine/limine-bios.sys"    "$ISO_ROOT/"
cp "$ROOT/limine/limine-bios-cd.bin" "$ISO_ROOT/"
cp "$ROOT/limine/limine-uefi-cd.bin" "$ISO_ROOT/"

# Build a FAT efi.img holding the UEFI bootloader (for UEFI El Torito boot)
EFI_IMG="$ISO_ROOT/efi.img"
dd if=/dev/zero of="$EFI_IMG" bs=1M count=30 status=none
mformat -i "$EFI_IMG" ::
mmd -i "$EFI_IMG" ::/EFI
mmd -i "$EFI_IMG" ::/EFI/BOOT
mcopy -i "$EFI_IMG" "$ISO_ROOT/EFI/BOOT/BOOTX64.EFI" ::/EFI/BOOT/BOOTX64.EFI

# Build the hybrid ISO
xorriso -as mkisofs \
    -b limine-bios-cd.bin \
    -no-emul-boot \
    -boot-load-size 4 \
    -boot-info-table \
    --efi-boot efi.img \
    -efi-boot-part \
    --efi-boot-image \
    "$ISO_ROOT" -o "$OUT"

echo "ISO built: $OUT"
ls -l "$OUT"
