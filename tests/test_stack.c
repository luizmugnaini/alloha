/// Stack memory allocator tests.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <alloha/stack.h>

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

#define HEADER_SIZE  sizeof(struct stack_header)
#define HEADER_ALIGN alloha_alignof(struct stack_header)

static void stack_offsets_reads_and_writes(void) {
    struct stack stack;
    usize        buf_size = 1024;
    u8*          buf      = (u8*)malloc(buf_size);

    stack_init(&stack, buf_size, buf);
    uptr buf_start_addr = (uptr)buf;

    // Create an array of `u64`.
    usize array1_len       = 70;
    usize array1_size      = array1_len * sizeof(u64);
    usize array1_alignment = sizeof(u64);
    u64*  array1           = (u64*)stack_alloc_aligned(&stack, array1_size, (u32)array1_alignment);

    // Write to `array1`.
    for (u32 idx = 0; idx < array1_len; idx++) {
        array1[idx] = 64 * (u64)idx;
    }

    // Check correctness of the `array1` offset.
    usize array1_expected_padding = HEADER_SIZE;
    usize array1_expected_offset  = array1_expected_padding;
    assert(stack.previous_offset == array1_expected_offset);

    uptr const array1_addr = buf_start_addr + (uptr)array1_expected_offset;

    // Check the correctness of the `array1` header.
    struct stack_header const* array1_header =
        (struct stack_header const*)(array1_addr - HEADER_SIZE);
    assert(array1_header->padding == array1_expected_padding);
    assert(array1_header->previous_offset == 0);

    // Manually read from `stack.buf` checking for the values of `array1`.
    for (usize idx = 0; idx < array1_len; idx++) {
        u64 const* actual = (u64 const*)(array1_addr + (uptr)(idx * array1_alignment));
        assert(*actual == 64 * (u64)idx);
    }

    // Check offset to the free memory in the stack.
    usize const after_array1_expected_offset = array1_expected_offset + array1_size;
    assert(stack.offset == after_array1_expected_offset);

    // Create an array of strings.
    usize array2_len       = 30;
    usize array2_size      = array2_len * sizeof(i32);
    usize array2_alignment = sizeof(i32);
    i32*  array2           = (i32*)stack_alloc_aligned(&stack, array2_size, (u32)array2_alignment);

    // Write to `array2`.
    i32 array2_constant_value = 123456;
    for (usize idx = 0; idx < array2_len; idx++) {
        array2[idx] = array2_constant_value;
    }

    // Check correctness of the `array2` offset.
    usize array2_alignment_modifier = after_array1_expected_offset % array2_alignment;
    usize array2_expected_padding =
        array2_alignment_modifier == 0 ? HEADER_SIZE
                                       : array2_alignment - array2_alignment_modifier + HEADER_SIZE;
    usize array2_expected_offset = after_array1_expected_offset + array2_expected_padding;
    assert(stack.previous_offset == array2_expected_offset);

    uptr array2_addr = buf_start_addr + (uptr)array2_expected_offset;

    // Check the correctness of the `array2` header.
    struct stack_header const* array2_header =
        (struct stack_header const*)(array2_addr - HEADER_SIZE);
    assert(array2_header->padding == array2_expected_padding);
    assert(array2_header->previous_offset == (usize)(array1_addr - buf_start_addr));

    // Manually read from `stack.buf` checking for the values of `array2`.
    for (usize idx = 0; idx < array2_len; idx++) {
        i32 const* actual = (i32*)(array2_addr + (uptr)(idx * array2_alignment));
        assert(*actual == array2_constant_value);
    }

    // Check offset to the free memory in the stack.
    usize after_array2_expected_offset = array2_addr + array2_size - buf_start_addr;
    assert(stack.offset == after_array2_expected_offset);

    free(buf);
    printf("Test `stack_offsets_reads_and_writes` passed.\n");
}

static void stack_memory_stress_and_free(void) {
    usize const buf_size = 2048;
    u8* const   buf      = (u8*)malloc(buf_size);

    struct stack stack          = stack_new(buf_size, buf);
    iptr         stack_buf_diff = (iptr)stack.buf;

    usize       a1_size      = 50 * sizeof(char*);
    usize       a1_alignment = sizeof(char*);
    char const* a1           = (char const*)stack_alloc_aligned(&stack, a1_size, (u32)a1_alignment);
    assert(a1);
    assert(((stack.previous_offset - HEADER_SIZE) % alloha_alignof(struct stack_header)) == 0);
    assert((stack.previous_offset % a1_alignment) == 0);
    assert((iptr)stack.previous_offset == (iptr)a1 - stack_buf_diff);

    usize a2_size      = 100 * sizeof(i32);
    usize a2_alignment = sizeof(i32);
    i32*  a2           = (i32*)stack_alloc_aligned(&stack, a2_size, (u32)a2_alignment);
    assert(a2);
    assert(((stack.previous_offset - HEADER_SIZE) % alloha_alignof(struct stack_header)) == 0);
    assert((stack.previous_offset % a2_alignment) == 0);
    assert((iptr)stack.previous_offset == (iptr)a2 - stack_buf_diff);

    usize a3_size      = 33 * sizeof(u64);
    usize a3_alignment = sizeof(u64);
    u64*  a3           = (u64*)stack_alloc_aligned(&stack, a3_size, (u32)a3_alignment);
    assert(a3);
    assert(((stack.previous_offset - HEADER_SIZE) % alloha_alignof(struct stack_header)) == 0);
    assert(stack.previous_offset % a3_alignment == 0);
    assert((iptr)stack.previous_offset == (iptr)a3 - stack_buf_diff);

    usize a4_size      = 49 * sizeof(u8);
    usize a4_alignment = sizeof(u8);
    u8*   a4           = (u8*)stack_alloc_aligned(&stack, a4_size, (u32)a4_alignment);
    assert(a4);
    assert(((stack.previous_offset - HEADER_SIZE) % HEADER_ALIGN) == 0);
    assert(stack.previous_offset % a4_alignment == 0);
    assert((iptr)stack.previous_offset == (iptr)a4 - stack_buf_diff);

    usize a5_size      = 8 * sizeof(unsigned);
    usize a5_alignment = sizeof(unsigned);
    u32*  a5           = (u32*)stack_alloc_aligned(&stack, a5_size, (u32)a5_alignment);
    assert(a5);
    assert(((stack.previous_offset - HEADER_SIZE) % HEADER_ALIGN) == 0);
    assert(stack.previous_offset % a5_alignment == 0);
    assert((iptr)stack.previous_offset == (iptr)a5 - stack_buf_diff);

    usize       a6_size      = 14 * sizeof(char*);
    usize       a6_alignment = sizeof(char*);
    char const* a6           = (char const*)stack_alloc_aligned(&stack, a6_size, (u32)a6_alignment);
    assert(a6);
    assert(((stack.previous_offset - HEADER_SIZE) % HEADER_ALIGN) == 0);
    assert(stack.previous_offset % a6_alignment == 0);
    assert((iptr)stack.previous_offset == (iptr)a6 - stack_buf_diff);

    // Free a6.
    assert(stack_pop(&stack));  // NOTE: a6 is now dangling.
    assert((iptr)stack.previous_offset == (iptr)a5 - stack_buf_diff);

    // Free a3, a4, and a5 (they should all be dangling after the free).
    assert(stack_clear_at(&stack, (u8*)a3));
    assert((iptr)stack.previous_offset == (iptr)a2 - stack_buf_diff);

    // Free both a2 and a1.
    stack_clear(&stack);
    assert(stack.previous_offset == 0);
    assert(stack.offset == 0);
    assert(stack.buf && stack.capacity);  // The memory should still be available.

    // Ensure we can allocate after freeing all blocks.
    i32* b1 = (i32*)stack_alloc_aligned(&stack, 80 * sizeof(i32), (u32)alloha_alignof(i32));
    assert(b1);

    f64* b2 = (f64*)stack_alloc_aligned(&stack, 80 * sizeof(f64), (u32)alloha_alignof(f64));
    assert(b2);

    free(buf);
    printf("Test `stack_stress_and_free` passed.\n");
}

static void test_stack(void) {
    stack_offsets_reads_and_writes();
    stack_memory_stress_and_free();
}

#if !defined(ALLOHA_TEST_NO_MAIN)
int main(void) {
    test_stack();
    return 0;
}
#endif
