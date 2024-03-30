/**
 * @brief Tests for the arena allocator.
 * @file test_arena.c
 * @author Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <alloha/arena.h>

typedef struct Foo {
    unsigned int x : 1;
    unsigned int y : 32;
    unsigned int z : 1;
} Foo;

// Check reads and writes.
void arena_memory_not_owned(void) {
    int          test_passed = 1;
    arena_t      arena;
    size_t const buf_size = 1024;
    void* const  buf      = malloc(buf_size);
    {
        arena_init(&arena, buf_size, buf);

        size_t const a1_length    = 128;
        size_t const a1_size      = a1_length * sizeof(int);
        size_t const a1_alignment = sizeof(int);
        int* const   a1           = (int*)arena_alloc_aligned(&arena, a1_size, a1_alignment);

        // Write to the acquired memory
        for (int i = 0; i < (int)a1_length; i++) {
            a1[i] = 1902 + i;
        }

        // Read from a1 directly.
        for (size_t idx = 0; idx < a1_length; idx++) {
            int const expected = 1902 + (int)idx;
            int const actual   = a1[idx];
            if (expected != actual) {
                fprintf(stderr, "(array indexing) Expected: %d; Actual: %d.\n", expected, actual);
                test_passed = 0;
            }
        }
    }
    free(buf);
    if (test_passed) {
        printf("Test `arena_memory_not_owned` passed.\n");
    } else {
        printf("Test `arena_memory_not_owned` failed.\n");
    }
}

// Checking correctness of the offsets for various allocations demanding different alignments.
void arena_check_offsets(void) {
    size_t const mem_size = 1024;
    u8*          mem      = malloc(mem_size);
    {
        arena_t arena = arena_new(mem_size, mem);

        size_t const   arr_u8_len       = 255;
        size_t const   arr_u8_size      = arr_u8_len * sizeof(uint8_t);
        size_t const   arr_u8_alignment = sizeof(uint8_t);
        uint8_t* const arr_u8 = arena_alloc_aligned(&arena, arr_u8_size, arr_u8_alignment);
        for (uint8_t c = 0; c < (uint8_t)arr_u8_len; c++) {
            arr_u8[c] = c;
        }
        assert(arena.offset == arr_u8_size);

        size_t const    arr_u32_len       = 80;
        size_t const    arr_u32_size      = arr_u32_len * sizeof(uint32_t);
        size_t const    arr_u32_alignment = sizeof(uint32_t);
        uint32_t* const arr_u32 =
            (uint32_t*)arena_alloc_aligned(&arena, arr_u32_size, arr_u32_alignment);
        for (uint32_t i = 0; i < (uint32_t)arr_u32_len; i++) {
            arr_u32[i] = i + 1000;
        }

        size_t const arr_u32_expected_alignment =
            arr_u32_alignment - (arr_u8_size % arr_u32_alignment);
        size_t const arr_u32_expected_offset =
            arr_u8_size + arr_u32_expected_alignment + arr_u32_size;
        assert(arena.offset == arr_u32_expected_offset);

        size_t const arr_foo_len       = 30;
        size_t const arr_foo_size      = arr_foo_len * sizeof(Foo);
        size_t const arr_foo_alignment = sizeof(Foo) + 4;
        Foo* const   arr_foo = (Foo*)arena_alloc_aligned(&arena, arr_foo_size, arr_foo_alignment);
        for (unsigned int i = 0; i < (unsigned int)arr_foo_len; i++) {
            Foo f;
            f.x = f.y = f.z = 1;
            arr_foo[i]      = f;
        }

        size_t const arr_foo_expected_alignment = arr_u32_expected_offset % arr_foo_alignment;
        size_t const arr_foo_expected_previous_offset =
            arr_u32_expected_offset + arr_foo_expected_alignment;
        size_t const arr_foo_expected_offset = arr_foo_expected_previous_offset + arr_foo_size;
        assert(arena.offset == arr_foo_expected_offset);
    }
    free(mem);
    printf("Test `arena_check_offsets` passed.\n");
}

int main(void) {
    arena_memory_not_owned();
    arena_check_offsets();
    return 0;
}
