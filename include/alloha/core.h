/**
 * @brief Common functions and variables used by the allocators.
 * @file core.h
 * @author Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
 */
#ifndef CORE_HEADER
#define CORE_HEADER

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif  // DEFAULT_ALIGNMENT

#define ALLOHA_TRUE 1
#define ALLOHA_FALSE 0

int is_power_of_two(unsigned const x);

#endif  // CORE_HEADER
