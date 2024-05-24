/// Implementation of the core utilities for the implementation of the allocators.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <alloha/core.h>

#include <assert.h>
#include <string.h>

usize usize_wrap_sub(usize lhs, usize rhs) {
    isize res = (isize)lhs - (isize)rhs;
    return (res <= 0) ? 0 : (usize)res;
}

void memory_copy(u8* dest, u8 const* src, usize size) {
    if (dest == NULL || src == NULL) {
        return;
    }

#if defined(DEBUG) || defined(ALLOHA_CHECK_MEMCPY_OVERLAP)
    uptr dest_addr = (uptr)dest;
    uptr src_addr  = (uptr)src;
    PSH_ASSERT_MSG(
        (dest_addr + size > src_addr) || (dest_addr < src_addr + size),
        "memcpy called but source and destination overlap, which produces UB");
#endif

    alloha_discard(memcpy(dest, src, size));
}

uptr align_forward(uptr ptr, u32 alignment) {
    assert(alloha_is_power_of_two(alignment) && "align_forward expected a power of two alignment");

    uptr mod = ptr & (alignment - 1);  // NOTE: Same as `ptr % al` (but faster).
    if (mod != 0) {
        // `ptr` is unaligned and we need to put it to the next aligned address.
        ptr += alignment - mod;
    }

    return ptr;
}

u32 padding_with_header(uptr ptr, u32 alignment, u32 header_size, u32 header_alignment) {
    assert(
        alloha_is_power_of_two(alignment) &&
        "padding_with_header expected the memory alignment to be a power of two");
    assert(
        alloha_is_power_of_two(header_alignment) &&
        "padding_with_header expected the header alignment to be a power of two");

    u32 padding = 0;
    u32 mod     = (u32)(ptr & ((uptr)alignment - 1));
    if (mod != 0) {
        padding += alignment - mod;
    }
    ptr += padding;

    // Compute the padding necessary for the header alignment.
    u32 mod_header = (u32)(ptr & ((uptr)header_alignment - 1));
    if (mod_header != 0) {
        padding += header_alignment - mod_header;
    }
    padding += header_size;

    return padding;
}
