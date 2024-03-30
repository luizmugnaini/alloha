/// Implementation of the core utilities for the implementation of the allocators.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
#include <alloha/core.h>

#include <assert.h>
#include <string.h>

bool is_power_of_two(u32 x) {
    return (x > 0) && !(x & (x - 1));
}

usize usize_wrap_sub(usize lhs, usize rhs) {
    isize const res = (isize)lhs - (isize)rhs;
    return (res <= 0) ? 0 : (usize)res;
}

void memory_copy(u8* dest, u8 const* src, usize size) {
    if (dest == NULL || src == NULL) {
        return;
    }

#if defined(DEBUG) || defined(ALLOHA_CHECK_MEMCPY_OVERLAP)
    auto const dest_addr = reinterpret_cast<uptr>(dest);
    auto const src_addr  = reinterpret_cast<uptr>(src);
    PSH_ASSERT_MSG(
        (dest_addr + size > src_addr) || (dest_addr < src_addr + size),
        "memcpy called but source and destination overlap, which produces UB");
#endif

    ALLOHA_UNUSED_RESULT(memcpy(dest, src, size));
}

uptr align_forward(uptr ptr, u32 alignment) {
    assert(is_power_of_two(alignment) && "align_forward expected a power of two alignment");

    uptr const al = (uptr)alignment;
    uptr const mod =
        ptr & (al - 1);  // NOTE: Same as `ptr % al` (but faster) since `al` is a power of two.
    if (mod != 0) {
        // `ptr` is unaligned and we need to put it to the next aligned address.
        ptr += al - mod;
    }

    return ptr;
}

u32 padding_with_header(uptr ptr, u32 alignment, u32 header_size, u32 header_alignment) {
    assert(
        is_power_of_two(alignment) &&
        "padding_with_header expected the memory alignment to be a power of two");
    assert(
        is_power_of_two(header_alignment) &&
        "padding_with_header expected the header alignment to be a power of two");

    u32       padding = 0;
    u32 const mod     = (u32)(ptr & ((uptr)alignment - 1));
    if (mod != 0) {
        padding += alignment - mod;
    }

    // Compute the padding necessary for the header alignment.
    ptr += padding;
    u32 const mod_header = (u32)(ptr & ((uptr)header_alignment - 1));
    if (mod_header != 0) {
        padding += header_alignment - mod_header;
    }
    padding += header_size;

    return padding;
}
