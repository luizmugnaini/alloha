# Alloha 🌺

This project contains the implementation of classic memory allocators from scratch, written in C.

## Building

The build system is based on CMake. You can use any of the major three compilers: Clang, GCC,
or MSVC. For instance, if you want to compile with Clang and use Ninja as your generator, you can
run

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=clang -G=Ninja
```

Now you can simply build the project with:

```bash
cmake --build build
```

## References and Similar Projects

- [Memory allocation strategies series](https://www.gingerbill.org/series/memory-allocation-strategies/), by gingerBill.
- [Memory management reference](https://www.memorymanagement.org/index.html).
- [mtrebi/memory-allocators](https://github.com/mtrebi/memory-allocators)
- [microsoft/mimalloc](https://github.com/microsoft/mimalloc)
