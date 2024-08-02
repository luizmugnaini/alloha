/// Common functions and variables used by the allocators.
///
/// Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#pragma once

#include <stdbool.h>
#include <stdint.h>

/// Unsigned integer type.
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint64_t usize;

/// Signed integer type.
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int64_t isize;

/// Memory-address types.
typedef uintptr_t uptr;
typedef intptr_t  iptr;

/// Floating-point types.
typedef float  f32;
typedef double f64;

/// The default alignment used when no specific alignment is specified.
#define ALLOHA_DEFAULT_ALIGNMENT (2 * sizeof(void*))

#define alloha_min(x, y) (((x) <= (y)) ? (x) : (y))
#define alloha_max(x, y) (((x) >= (y)) ? (x) : (y))

#define alloha_discard(res) ((void)res)

#define alloha_alignof(x) _Alignof(x)

/// Check if a given number is a power of two.
#define alloha_is_power_of_two(x) (((x) > 0) && !((x) & ((x)-1)))

/// Subtract two values of unsigned size type avoiding underflow.
usize usize_wrap_sub(usize lhs, usize rhs);

#define alloha_ptr_add(ptr, offset) ((ptr) ? ((ptr) + (offset)) : NULL)
#define alloha_ptr_sub(ptr, offset) ((ptr) ? ((ptr) - (offset)) : NULL)

/// Safely copy memory from one region to the other.
void memory_copy(u8* dest, u8 const* src, usize size);

/// Computes the next address with the required alignment.
///
/// Parameters:
///     * `ptr`: Current memory address.
///     * `alignment`: The alignment requirement that should be used to compute the next address.
///
/// Return: Next address, with respect to `ptr` that satisfies the required alignment.
uptr align_forward(uptr ptr, u32 alignment);

/// Given an address, computes the padding needed to reach the next address that fits the header and
/// satisfies the memory alignment requirements imposed by the header and memory.
///
/// Parameters:
///     * `ptr`: Current address.
///     * `alignment`: Memory alignment requirement for the memory being allocated.
///     * `header_size`: Size of the header associated with the new block of memory to be allocated.
///     * `header_alignment`: Memory alignment required for the header.
///
/// Return: Memory padding such that when we offset `ptr` by the padding we obtain a space that fits
///         the header and satisfies all alignment requirements.
u32 padding_with_header(uptr ptr, u32 alignment, u32 header_size, u32 header_alignment);
