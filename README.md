# Alloha 🌺

This project contains the implementation of classic memory allocators from scratch, written in C.

# Development

The library has a bundled compilation unit `src/all.c` which may be used if you wish to compile as
a unity build. This is as simple as, e.g.:
```sh
# Build static library.
clang -c -std=c11 -Iinclude src/all.c -o liballoha.o && llvm-ar rc liballoha.a liballoha.o
# Build all library tests.
clang -std=c11 -Iinclude tests/test_all.c -o test
```

Another option is to use the `build.lua` script, which will manage to build the project with many
custom options that may be viewed in the file itself. With that said, Lua is, optionally, the only
dependency of the whole project - being only required if you want the convenience of running the build
script.

## References and Similar Projects

- [Memory allocation strategies series](https://www.gingerbill.org/series/memory-allocation-strategies/), by gingerBill.
- [Memory management reference](https://www.memorymanagement.org/index.html).
- [mtrebi/memory-allocators](https://github.com/mtrebi/memory-allocators)
- [microsoft/mimalloc](https://github.com/microsoft/mimalloc)
