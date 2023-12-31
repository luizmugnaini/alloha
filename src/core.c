/**
 * @brief Implementation of the core utilities for the implementation of the allocators.
 * @file core.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/core.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

int is_power_of_two(unsigned const x) {
    return x > 0 && !(x & (x - 1));
}

uintptr_t align_forward(uintptr_t ptr, size_t const alignment) {
    assert(is_power_of_two(alignment) && "align_forward expected a power of two alignment");

    uintptr_t const al = (uintptr_t)alignment;
    uintptr_t const mod =
        ptr & (al - 1);  // NOTE: Same as `ptr % al` (but faster) since `al` is a power of two.
    if (mod != 0) {
        // `ptr` is unaligned and we need to put it to the next aligned address.
        ptr += al - mod;
    }

    return ptr;
}

size_t padding_with_header(
    uintptr_t ptr,
    size_t const alignment,
    size_t const header_size,
    size_t const header_alignment) {
    assert(
        is_power_of_two(alignment) &&
        "padding_with_header expected the memory alignment to be a power of two");
    assert(
        is_power_of_two(header_alignment) &&
        "padding_with_header expected the header alignment to be a power of two");

    uintptr_t padding = 0;
    uintptr_t const al = (uintptr_t)alignment;
    uintptr_t const mod = ptr & (al - 1);
    if (mod != 0) {
        padding += al - mod;
    }

    // Compute the padding necessary for the header alignment.
    ptr += padding;
    uintptr_t const al_header = (uintptr_t)header_alignment;
    uintptr_t const mod_header = ptr & (al_header - 1);
    if (mod_header != 0) {
        padding += al_header - mod_header;
    }
    padding += header_size;

    return (size_t)padding;
}
