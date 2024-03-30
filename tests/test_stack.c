#include <alloha/stack.h>

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

void stack_offsets_reads_and_writes(void) {
    stack_t     stack;
    usize const buf_size = 1024;
    u8* const   buf      = (u8*)malloc(buf_size);
    {
        stack_init(&stack, buf_size, buf);
        uptr const buf_start_addr = (uptr)buf;

        // Create an array of `u64`.
        usize const array1_len       = 70;
        usize const array1_size      = array1_len * sizeof(u64);
        usize const array1_alignment = sizeof(u64);
        u64* const  array1 = (u64*)stack_alloc_aligned(&stack, array1_size, array1_alignment);

        // Write to `array1`.
        for (u32 idx = 0; idx < array1_len; idx++) {
            array1[idx] = 64 * (u64)idx;
        }

        // Check correctness of the `array1` offset.
        usize const array1_expected_padding = sizeof(stack_header_t);
        usize const array1_expected_offset  = array1_expected_padding;
        assert(stack.previous_offset == array1_expected_offset);

        uptr const array1_addr = buf_start_addr + (uptr)array1_expected_offset;

        // Check the correctness of the `array1` header.
        stack_header_t const* const array1_header =
            (stack_header_t const*)(array1_addr - sizeof(stack_header_t));
        assert(array1_header->padding == array1_expected_padding);
        assert(array1_header->previous_offset == 0);

        // Manually read from `stack.buf` checking for the values of `array1`.
        for (usize idx = 0; idx < array1_len; idx++) {
            u64 const* const actual = (u64 const*)(array1_addr + (uptr)(idx * array1_alignment));
            assert(*actual == 64 * (u64)idx);
        }

        // Check offset to the free memory in the stack.
        usize const after_array1_expected_offset = array1_expected_offset + array1_size;
        assert(stack.offset == after_array1_expected_offset);

        // Create an array of strings.
        usize const array2_len       = 30;
        usize const array2_size      = array2_len * sizeof(i32);
        usize const array2_alignment = sizeof(i32);
        i32* const  array2 = (i32*)stack_alloc_aligned(&stack, array2_size, array2_alignment);

        // Write to `array2`.
        i32 const array2_constant_value = 123456;
        for (usize idx = 0; idx < array2_len; idx++) {
            array2[idx] = array2_constant_value;
        }

        // Check correctness of the `array2` offset.
        usize const array2_alignment_modifier = after_array1_expected_offset % array2_alignment;
        usize const array2_expected_padding =
            array2_alignment_modifier == 0
                ? sizeof(stack_header_t)
                : array2_alignment - array2_alignment_modifier + sizeof(stack_header_t);
        usize const array2_expected_offset = after_array1_expected_offset + array2_expected_padding;
        assert(stack.previous_offset == array2_expected_offset);

        uptr const array2_addr = buf_start_addr + (uptr)array2_expected_offset;

        // Check the correctness of the `array2` header.
        stack_header_t const* const array2_header =
            (stack_header_t const*)(array2_addr - sizeof(stack_header_t));
        assert(array2_header->padding == array2_expected_padding);
        assert(array2_header->previous_offset == (usize)(array1_addr - buf_start_addr));

        // Manually read from `stack.buf` checking for the values of `array2`.
        for (usize idx = 0; idx < array2_len; idx++) {
            i32 const* const actual = (i32*)(array2_addr + (uptr)(idx * array2_alignment));
            assert(*actual == array2_constant_value);
        }

        // Check offset to the free memory in the stack.
        usize const after_array2_expected_offset = array2_addr + array2_size - buf_start_addr;
        assert(stack.offset == after_array2_expected_offset);
    }
    free(buf);
    printf("Test `stack_offsets_reads_and_writes` passed.\n");
}

void stack_memory_stress_and_free(void) {
    usize const buf_size = 2048;
    u8* const   buf      = (u8*)malloc(buf_size);
    {
        stack_t    stack          = stack_new(buf_size, buf);
        iptr const stack_buf_diff = (iptr)stack.buf;

        usize const a1_size      = 50 * sizeof(str_ptr_t);
        usize const a1_alignment = sizeof(str_ptr_t);
        str_ptr_t   a1           = (char const*)stack_alloc_aligned(&stack, a1_size, a1_alignment);
        assert(a1);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
        assert((stack.previous_offset % a1_alignment) == 0);
        assert((iptr)stack.previous_offset == (iptr)a1 - stack_buf_diff);

        usize const a2_size      = 100 * sizeof(i32);
        usize const a2_alignment = sizeof(i32);
        i32*        a2           = (i32*)stack_alloc_aligned(&stack, a2_size, a2_alignment);
        assert(a2);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
        assert((stack.previous_offset % a2_alignment) == 0);
        assert((iptr)stack.previous_offset == (iptr)a2 - stack_buf_diff);

        usize const a3_size      = 33 * sizeof(u64);
        usize const a3_alignment = sizeof(u64);
        u64*        a3           = (u64*)stack_alloc_aligned(&stack, a3_size, a3_alignment);
        assert(a3);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
        assert(stack.previous_offset % a3_alignment == 0);
        assert((iptr)stack.previous_offset == (iptr)a3 - stack_buf_diff);

        usize const a4_size      = 49 * sizeof(u8);
        usize const a4_alignment = sizeof(u8);
        u8*         a4           = (u8*)stack_alloc_aligned(&stack, a4_size, a4_alignment);
        assert(a4);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
        assert(stack.previous_offset % a4_alignment == 0);
        assert((iptr)stack.previous_offset == (iptr)a4 - stack_buf_diff);

        usize const a5_size      = 8 * sizeof(unsigned);
        usize const a5_alignment = sizeof(unsigned);
        u32*        a5           = (u32*)stack_alloc_aligned(&stack, a5_size, a5_alignment);
        assert(a5);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
        assert(stack.previous_offset % a5_alignment == 0);
        assert((iptr)stack.previous_offset == (iptr)a5 - stack_buf_diff);

        usize const a6_size      = 14 * sizeof(str_ptr_t);
        usize const a6_alignment = sizeof(str_ptr_t);
        str_ptr_t   a6           = (char const*)stack_alloc_aligned(&stack, a6_size, a6_alignment);
        assert(a6);
        assert(((stack.previous_offset - sizeof(stack_header_t)) % alignof(stack_header_t)) == 0);
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
        i32* const b1 = (i32*)stack_alloc_aligned(&stack, 80 * sizeof(i32), alignof(i32));
        assert(b1);

        f64* const b2 = (f64*)stack_alloc_aligned(&stack, 80 * sizeof(f64), alignof(f64));
        assert(b2);
    }
    free(buf);
    printf("Test `stack_stress_and_free` passed.\n");
}

int main(void) {
    stack_offsets_reads_and_writes();
    stack_memory_stress_and_free();
    return 0;
}
