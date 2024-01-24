# Alloha 🌺

This project contains the implementation of classic memory allocators from scratch, written in C.

## Building

Our build system is based on CMake, and the C compiler of choice is Clang. In order to generate the CMake build system, you may
run the following:

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=clang
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
