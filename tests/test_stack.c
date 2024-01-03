#include <alloha/stack.h>

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

size_t const k_stack_header_size = sizeof(stack_alloc_header_t);
size_t const k_stack_header_alignment = alignof(stack_alloc_header_t);

void stack_offsets_reads_and_writes(void) {
    stack_alloc_t stack;
    size_t const buf_size = 1024;
    void* const buf = malloc(buf_size);
    stack_init(&stack, buf, buf_size);
    uintptr_t const buf_start_addr = (uintptr_t)buf;

    assert(!stack.memory_owner);

    // Create an array of `uint64_t`.
    size_t const array1_len = 70;
    size_t const array1_size = array1_len * sizeof(uint64_t);
    size_t const array1_alignment = sizeof(uint64_t);
    uint64_t* const array1 = (uint64_t*)stack_alloc_aligned(&stack, array1_size, array1_alignment);

    // Write to `array1`.
    for (unsigned idx = 0; idx < array1_len; idx++) {
        array1[idx] = 64 * (uint64_t)idx;
    }

    // Check correctness of the `array1` offset.
    size_t const array1_expected_padding = k_stack_header_size;
    size_t const array1_expected_offset = array1_expected_padding;
    assert(stack.previous_offset == array1_expected_offset);

    uintptr_t const array1_addr = buf_start_addr + (uintptr_t)array1_expected_offset;

    // Check the correctness of the `array1` header.
    stack_alloc_header_t const* const array1_header =
        (stack_alloc_header_t const*)(array1_addr - (uintptr_t)k_stack_header_size);
    assert(array1_header->padding == array1_expected_padding);
    assert(array1_header->previous_offset == 0);

    // Manually read from `stack.buf` checking for the values of `array1`.
    for (size_t idx = 0; idx < array1_len; idx++) {
        uint64_t const* const actual =
            (uint64_t const*)(array1_addr + (uintptr_t)(idx * array1_alignment));
        assert(*actual == 64 * (uint64_t)idx);
    }

    // Check offset to the free memory in the stack.
    size_t const after_array1_expected_offset = array1_expected_offset + array1_size;
    assert(stack.offset == after_array1_expected_offset);

    // Create an array of strings.
    size_t const array2_len = 30;
    size_t const array2_size = array2_len * sizeof(int);
    size_t const array2_alignment = sizeof(int);
    int* const array2 = (int*)stack_alloc_aligned(&stack, array2_size, array2_alignment);

    // Write to `array2`.
    int const array2_constant_value = 123456;
    for (size_t idx = 0; idx < array2_len; idx++) {
        array2[idx] = array2_constant_value;
    }

    // Check correctness of the `array2` offset.
    size_t const array2_alignment_modifier = after_array1_expected_offset % array2_alignment;
    size_t const array2_expected_padding =
        array2_alignment_modifier == 0
            ? k_stack_header_size
            : array2_alignment - array2_alignment_modifier + k_stack_header_size;
    size_t const array2_expected_offset = after_array1_expected_offset + array2_expected_padding;
    assert(stack.previous_offset == array2_expected_offset);

    uintptr_t const array2_addr = buf_start_addr + (uintptr_t)array2_expected_offset;

    // Check the correctness of the `array2` header.
    stack_alloc_header_t const* const array2_header =
        (stack_alloc_header_t const*)(array2_addr - (uintptr_t)k_stack_header_size);
    assert(array2_header->padding == array2_expected_padding);
    assert(array2_header->previous_offset == (size_t)(array1_addr - buf_start_addr));

    // Manually read from `stack.buf` checking for the values of `array2`.
    for (size_t idx = 0; idx < array2_len; idx++) {
        int const* const actual = (int const*)(array2_addr + (uintptr_t)(idx * array2_alignment));
        assert(*actual == array2_constant_value);
    }

    // Check offset to the free memory in the stack.
    size_t const after_array2_expected_offset = array2_addr + array2_size - buf_start_addr;
    assert(stack.offset == after_array2_expected_offset);

    free(buf);
    printf("Test `stack_offsets_reads_and_writes` passed.\n");
}

void stack_owned_memory(void) {
    stack_alloc_t stack = stack_create(512);
    assert(stack.memory_owner);
    unsigned* const array =
        (unsigned*)stack_alloc_aligned(&stack, 30 * sizeof(unsigned), sizeof(unsigned));
    (void)array;
    stack_destroy(&stack);
    printf("Test `stack_owned_memory` passed.\n");
}

void stack_memory_stress_and_free(void) {
    stack_alloc_t stack = stack_create(2048);
    ptrdiff_t const stack_buf_diff = (ptrdiff_t)stack.buf;

    size_t const a1_size = 50 * sizeof(char const*);
    size_t const a1_alignment = sizeof(char const*);
    char const* a1 = (char const*)stack_alloc_aligned(&stack, a1_size, a1_alignment);
    assert(a1);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a1_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a1 - stack_buf_diff);

    size_t const a2_size = 100 * sizeof(int);
    size_t const a2_alignment = sizeof(int);
    int* a2 = (int*)stack_alloc_aligned(&stack, a2_size, a2_alignment);
    assert(a2);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a2_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a2 - stack_buf_diff);

    size_t const a3_size = 33 * sizeof(uint64_t);
    size_t const a3_alignment = sizeof(uint64_t);
    uint64_t* a3 = (uint64_t*)stack_alloc_aligned(&stack, a3_size, a3_alignment);
    assert(a3);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a3_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a3 - stack_buf_diff);

    size_t const a4_size = 49 * sizeof(uint8_t);
    size_t const a4_alignment = sizeof(uint8_t);
    uint8_t* a4 = (uint8_t*)stack_alloc_aligned(&stack, a4_size, a4_alignment);
    assert(a4);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a4_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a4 - stack_buf_diff);

    size_t const a5_size = 8 * sizeof(unsigned);
    size_t const a5_alignment = sizeof(unsigned);
    unsigned* a5 = (unsigned*)stack_alloc_aligned(&stack, a5_size, a5_alignment);
    assert(a5);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a5_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a5 - stack_buf_diff);

    size_t const a6_size = 14 * sizeof(char const*);
    size_t const a6_alignment = sizeof(char const*);
    char const* a6 = (char const*)stack_alloc_aligned(&stack, a6_size, a6_alignment);
    assert(a6);
    assert((stack.previous_offset - k_stack_header_size) % k_stack_header_alignment == 0);
    assert(stack.previous_offset % a6_alignment == 0);
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a6 - stack_buf_diff);

    // Free a6.
    stack_pop(&stack);  // NOTE: a6 is now dangling.
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a5 - stack_buf_diff);

    // Free a3, a4, and a5 (they should all be dangling after the free).
    assert(stack_free(&stack, (void*)a3));
    assert((ptrdiff_t)stack.previous_offset == (ptrdiff_t)a2 - stack_buf_diff);

    // Free both a2 and a1.
    stack_free_all(&stack);
    assert(stack.previous_offset == 0);
    assert(stack.offset == 0);
    assert(stack.buf && stack.capacity);  // The memory should still be available.

    // Ensure we can allocate after freeing all blocks.
    int* const b1 = (int*)stack_alloc_aligned(&stack, 80 * sizeof(int), sizeof(int));
    double* const b2 = (double*)stack_alloc_aligned(&stack, 80 * sizeof(double), sizeof(double));
    assert(b1);
    assert(b2);

    // Destroy and ensure all resources are cleaned.
    stack_destroy(&stack);
    assert(!stack.buf);
    assert(stack.capacity == 0);
    assert(stack.offset == 0);
    assert(stack.previous_offset == 0);
    assert(!stack.memory_owner);

    printf("Test `stack_stress_and_free` passed.\n");
}

int main(void) {
    stack_offsets_reads_and_writes();
    stack_owned_memory();
    stack_memory_stress_and_free();
    return 0;
}
