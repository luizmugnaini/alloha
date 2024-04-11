/// Stack memory allocator implementation.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <alloha/stack.h>

#include <alloha/core.h>
#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

stack_t stack_new(usize capacity, u8* buf) {
    if (capacity != 0) {
        assert(buf && "stack_new called with inconsistent data: non-null capacity and null buffer");
    }
    return (stack_t){
        .buf             = buf,
        .capacity        = capacity,
        .offset          = 0,
        .previous_offset = 0,
    };
}

void stack_init(stack_t* restrict stack, usize capacity, u8* restrict buf) {
    if (!stack) {
        return;
    }
    if (capacity != 0) {
        assert(
            buf && "stack_init called with inconsistent data: non-null capacity and null buffer");
    }
    stack->buf             = buf;
    stack->capacity        = capacity;
    stack->offset          = 0;
    stack->previous_offset = 0;
}

u8* stack_alloc_aligned(stack_t* const stack, size_t const size, size_t const alignment) {
    if (!stack || stack->capacity == 0 || size == 0) {
        return NULL;
    }

    u8* const   free_mem           = ptr_add(stack->buf, stack->offset);
    usize const available_capacity = usize_wrap_sub(stack->capacity, stack->offset);

    u32 const padding = padding_with_header(
        (uptr)free_mem,
        alignment,
        sizeof(stack_header_t),
        alignof(stack_header_t));
    usize const required_size = (usize)padding + size;

    if (required_size > available_capacity) {
        fprintf(
            stderr,
            "Unable to allocate %zu bytes of memory (%zu bytes required), only %zu "
            "remaining.\n",
            size,
            padding + size,
            usize_wrap_sub(stack->capacity, stack->offset));
        return 0;
    }

    u8* const new_block = ptr_add(free_mem, padding);

    // Write to the header associated with the new block of memory.
    stack_header_t* const new_header = (stack_header_t*)ptr_sub(new_block, sizeof(stack_header_t));
    new_header->padding              = padding;
    new_header->capacity             = size;
    new_header->previous_offset      = stack->previous_offset;

    // Update the stack offsets.
    stack->previous_offset = stack->offset + padding;
    stack->offset += required_size;

    return new_block;
}

u8* stack_alloc(stack_t* const stack, usize const size) {
    return stack_alloc_aligned(stack, size, ALLOHA_DEFAULT_ALIGNMENT);
}

bool stack_pop(stack_t* stack) {
    if (!stack || stack->offset == 0) {
        return false;
    }

    // Find info about the current top memory block.
    u8 const* const             top        = ptr_add(stack->buf, stack->previous_offset);
    stack_header_t const* const top_header = (stack_header_t*)ptr_sub(top, sizeof(stack_header_t));

    // Update the stack.
    stack->offset          = stack->previous_offset - top_header->padding;
    stack->previous_offset = top_header->previous_offset;
    return true;
}

bool stack_clear_at(stack_t* restrict stack, u8* restrict block) {
    if (!stack || !block) {
        return false;
    }

    if (!(stack->buf <= block && block < ptr_add(stack->buf, stack->capacity))) {
        fprintf(
            stderr,
            "stack_free requested to free a block of memory outside of the range of the given "
            "stack allocator.\n");
        return ALLOHA_FALSE;
    }

    if (block >= ptr_add(stack->buf, stack->previous_offset)) {
        fprintf(stderr, "stack_free requested to free an already free memory block");
        return ALLOHA_FALSE;
    }

    stack_header_t const* const block_header =
        (stack_header_t const*)ptr_sub(block, sizeof(stack_header_t));
    stack->offset =
        usize_wrap_sub(usize_wrap_sub((uptr)block, block_header->padding), (usize)stack->buf);
    stack->previous_offset = block_header->previous_offset;

    return ALLOHA_TRUE;
}

void stack_clear(stack_t* stack) {
    if (!stack) {
        return;
    }
    stack->offset          = 0;
    stack->previous_offset = 0;
}
