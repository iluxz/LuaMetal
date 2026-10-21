# Contributing to LuaMetal

Thanks for your interest in contributing! 🎉

Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before participating.

## Getting started

1. Fork the repository
2. Clone your fork:
   ```powershell
   git clone https://github.com/<your-username>/LuaMetal.git
   cd LuaMetal
   ```
3. Create a branch for your change:
   ```powershell
   git checkout -b my-feature
   ```

## Building

See the [README](README.md#building) for toolchain setup. To build:

```powershell
.\build2.ps1
```

The build must succeed (ends with `BUILD OK`) before you open a PR.

## What to work on

Check the [roadmap](README.md#status) and open issues before starting big work.
Good first areas:

- **Kernel / shim** — new syscalls, better allocator, drivers
- **Lua integration** — load `.lua` files from disk, expose kernel APIs to Lua
- **Boot / build** — portability, faster builds, CI improvements

## Coding style

- C: match the existing style (tabs/spacing, brace style, naming)
- Keep the kernel freestanding — **no libc, no Linux headers**
- Prefer small, focused commits with clear messages

## Pull requests

1. Make sure the build passes locally
2. Update docs if you changed behavior
3. Open a PR with a clear description of what and why
4. Be ready to iterate on review feedback

## License

By contributing, you agree that your contributions will be licensed under the
[MIT License](LICENSE) that covers this project.
