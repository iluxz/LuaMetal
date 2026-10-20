$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$qemu = "C:\Program Files\qemu"
$llvm = "C:\Users\Can\AppData\Local\Microsoft\WinGet\Packages\MartinStorsjo.LLVM-MinGW.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\llvm-mingw-20260616-ucrt-x86_64\bin"
$clang = "$llvm\clang.exe"
$lld = "$llvm\ld.lld.exe"
$build = "$root\build"
$esp = "$build\esp"

New-Item -ItemType Directory -Path "$esp\EFI\BOOT" -Force | Out-Null

& $clang -target x86_64-unknown-none -ffreestanding -fno-stack-protector -fno-pic -fno-pie -mno-red-zone -O2 -I"$root\limine" -c "$root\kernel.c" -o "$build\kernel.o"
if ($LASTEXITCODE -ne 0) { throw "compile failed" }

& $lld -nostdlib -T "$root\linker.ld" "$build\kernel.o" -o "$build\kernel.elf"
if ($LASTEXITCODE -ne 0) { throw "link failed" }

Copy-Item "$root\limine\BOOTX64.EFI" "$esp\EFI\BOOT\BOOTX64.EFI" -Force
Copy-Item "$build\kernel.elf" "$esp\kernel.elf" -Force
Copy-Item "$root\limine.conf" "$esp\limine.conf" -Force

$vars = "$build\ovmf_vars.fd"
if (-not (Test-Path $vars)) { Copy-Item "$qemu\share\edk2-i386-vars.fd" $vars }

$espQ = $esp -replace '\\','/'
& "$qemu\qemu-system-x86_64.exe" -machine q35 -m 512M `
    -drive "if=pflash,format=raw,readonly=on,file=$($qemu -replace '\\','/')/share/edk2-x86_64-code.fd" `
    -drive "if=pflash,format=raw,file=$($vars -replace '\\','/')" `
    -drive "format=raw,file=fat:rw:$espQ" `
    -serial stdio -no-reboot
