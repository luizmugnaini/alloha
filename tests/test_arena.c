/// Arena memory allocator tests.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <alloha/arena.h>
#include <alloha/core.h>

struct foo {
    u32 x : 1;
    u32 y : 32;
    u32 z : 1;
};

// Check reads and writes.
void arena_memory_not_owned(void) {
    int          test_passed = 1;
    struct arena arena;
    usize        buf_size = 1024;
    u8*          buf      = (u8*)malloc(buf_size);

    arena_init(&arena, buf_size, buf);

    usize a1_length    = 128;
    usize a1_size      = a1_length * sizeof(int);
    usize a1_alignment = sizeof(int);
    int*  a1           = (int*)arena_alloc_aligned(&arena, a1_size, a1_alignment);

    // Write to the acquired memory
    for (int i = 0; i < (int)a1_length; i++) {
        a1[i] = 1902 + i;
    }

    // Read from a1 directly.
    for (usize idx = 0; idx < a1_length; idx++) {
        int expected = 1902 + (int)idx;
        int actual   = a1[idx];
        if (expected != actual) {
            fprintf(stderr, "(array indexing) Expected: %d; Actual: %d.\n", expected, actual);
            test_passed = 0;
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
    usize mem_size = 1024;
    u8*   mem      = (u8*)malloc(mem_size);

    struct arena arena = arena_new(mem_size, mem);

    usize arr_u8_len       = 255;
    usize arr_u8_size      = arr_u8_len * sizeof(u8);
    usize arr_u8_alignment = sizeof(u8);
    u8*   arr_u8           = arena_alloc_aligned(&arena, arr_u8_size, arr_u8_alignment);
    for (u8 c = 0; c < arr_u8_len; c++) {
        arr_u8[c] = c;
    }
    assert(arena.offset == arr_u8_size);

    usize arr_u32_len       = 80;
    usize arr_u32_size      = arr_u32_len * sizeof(uint32_t);
    usize arr_u32_alignment = sizeof(uint32_t);
    u32*  arr_u32           = (u32*)arena_alloc_aligned(&arena, arr_u32_size, arr_u32_alignment);
    for (u32 i = 0; i < arr_u32_len; i++) {
        arr_u32[i] = i + 1000;
    }

    usize arr_u32_expected_alignment = arr_u32_alignment - (arr_u8_size % arr_u32_alignment);
    usize arr_u32_expected_offset    = arr_u8_size + arr_u32_expected_alignment + arr_u32_size;
    assert(arena.offset == arr_u32_expected_offset);

    usize       arr_foo_len       = 30;
    usize       arr_foo_size      = arr_foo_len * sizeof(struct foo);
    usize       arr_foo_alignment = sizeof(struct foo) + 4;
    struct foo* arr_foo = (struct foo*)arena_alloc_aligned(&arena, arr_foo_size, arr_foo_alignment);
    for (u32 i = 0; i < arr_foo_len; i++) {
        struct foo f;
        f.x = f.y = f.z = 1;
        arr_foo[i]      = f;
    }

    usize arr_foo_expected_alignment       = arr_u32_expected_offset % arr_foo_alignment;
    usize arr_foo_expected_previous_offset = arr_u32_expected_offset + arr_foo_expected_alignment;
    usize arr_foo_expected_offset          = arr_foo_expected_previous_offset + arr_foo_size;
    assert(arena.offset == arr_foo_expected_offset);

    free(mem);
    printf("Test `arena_check_offsets` passed.\n");
}

int main(void) {
    arena_memory_not_owned();
    arena_check_offsets();
    return 0;
}
