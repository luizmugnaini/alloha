/**
 * @brief Implementation of the core utilities for the implementation of the allocators.
 * @file core.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/core.h>

int is_power_of_two(unsigned const x) {
    return x > 0 && !(x & (x - 1));
}
