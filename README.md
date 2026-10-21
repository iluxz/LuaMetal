# LuaMetal

> An operating system written in Lua, made from scratch — Lua 5.4 running on x86_64.

[![CI](https://github.com/iluxz/LuaMetal/actions/workflows/build.yml/badge.svg)](https://github.com/iluxz/LuaMetal/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Lua](https://img.shields.io/badge/Lua-5.4.8-2C2D72.svg)](https://www.lua.org/)

**LuaMetal** is a from-scratch operating system for x86_64 UEFI machines — a
tiny freestanding C kernel with **Lua 5.4.8** compiled directly into it. The
kernel boots via [Limine](https://github.com/limine-bootloader/limine) and drops
straight into a real Lua interpreter.

The goal: an OS where the system language *is* Lua, all the way down.

## Status

LuaMetal is early. Very early. Here's the roadmap:

- [x] **Milestone 1** — UEFI boot via Limine, serial output, framebuffer
- [x] **Milestone 2** — Lua 5.4.8 interpreter running freestanding in the kernel
- [x] **Milestone 3** — load and run real `.lua` files from disk as the OS's init program
- [ ] **Milestone 4** — desktop environment (a modern one, not 2011 Ubuntu) — *in progress: window, taskbar and font renderer all draw in Lua*
- [ ] **Milestone 5** — userspace, drivers, the works

## Hello, world

The kernel loads `init.lua` from disk at boot and runs it as the OS's first
program — edit the file, reboot, done:

```lua
print("hello from lua!")

for i = 1, 5 do
  print("count " .. i)
end

print("2 + 2 = " .. 2 + 2)
```

Output on the serial console:

```
hello from lua!
count 1
count 2
count 3
count 4
count 5
2 + 2 = 4
```

## Architecture

| Layer        | What it is                                                    |
|--------------|---------------------------------------------------------------|
| Bootloader   | Limine (UEFI), higher-half kernel at `0xFFFFFFFF80100000`     |
| Kernel       | Freestanding C, x86_64, no libc                               |
| Runtime      | Lua 5.4.8, compiled freestanding into the kernel              |
| Shim         | Custom minimal libc (serial, bump alloc, setjmp, math)        |
| Framebuffer  | Limine framebuffer protocol                                   |

## Building

Requirements:

- LLVM/Clang with `x86_64-unknown-none` target (e.g. [llvm-mingw](https://github.com/mstorsjo/llvm-mingw) or system clang + lld)
- [QEMU](https://www.qemu.org/) + OVMF for testing (optional)

```powershell
# Windows
.\build2.ps1
```

The build produces `build/kernel.elf` and stages a bootable ESP in `build/esp`.

## Running in QEMU

```powershell
qemu-system-x86_64 `
  -machine q35 -m 2G `
  -cpu qemu64 `
  -bios build/ovmf_code.fd `
  -drive format=raw,file=fat:rw:build/esp `
  -serial file:build/serial.log `
  -display none
```

Then check `build/serial.log` for the Lua output.

## Project layout

```
.
├── kernel.c        # kernel entry, SSE init, framebuffer, calls lua_run()
├── linker.ld       # higher-half linker script
├── limine.conf     # Limine bootloader config
├── shim/           # freestanding libc shim + Lua glue
│   ├── serial.c/h  # COM1 serial driver
│   ├── libc_shim.c # ctype/string/mem/alloc/strtod/snprintf/setjmp/math/stdio
│   ├── lua_run.c   # creates the Lua state, runs the embedded script
│   └── lua_overrides.h  # routes Lua's print/IO to serial
├── lua-5.4.8/      # vendored Lua 5.4.8 source
└── limine/         # Limine bootloader binaries + headers
```

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) and our
[Code of Conduct](CODE_OF_CONDUCT.md).

## License

[MIT](LICENSE) — do what you want, just keep the copyright notice.

---

Made with 💜 by [iluxz](https://github.com/iluxz)
