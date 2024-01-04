/**
 * @brief Common functions and variables used by the allocators.
 * @file core.h
 * @author Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
 */
#ifndef CORE_HEADER
#define CORE_HEADER

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uintptr_t

#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#define ALLOHA_TRUE 1
#define ALLOHA_FALSE 0

/**
 * Check if a given number is a power of two.
 *
 * @param x Number to be checked to be a power of two.
 *
 * @return Returns 1 if `x` is a power of two, 0 otherwise.
 */
int is_power_of_two(unsigned const x);

/**
 * Given an address, compute the next address that satisfies the required alignment.
 *
 * @param ptr Current address.
 * @param alignment Required memory alignment.
 *
 * @return Nearest address satisfying the alignment.
 */
uintptr_t align_forward(uintptr_t ptr, size_t const alignment);

/**
 * Given an address, computes the padding needed to reach the next address that fits the header and
 * satisfies the memory alignment requirements imposed by the header and memory.
 *
 * @param ptr Current address.
 * @param aligment Memory alignment requirement for the memory being allocated.
 * @param header_size Size of the header associated with the new block of memory to be allocated.
 * @param header_alignment Memory alignment required for the header.
 *
 * @return Memory padding such that when we offset `ptr` by the padding we obtain a space that fits
 *         the header and satisfies all alignment requirements.
 */
size_t padding_with_header(
    uintptr_t ptr, size_t const alignment, size_t const header_size, size_t const header_alignment);

#endif  // CORE_HEADER
