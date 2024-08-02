/// Stack memory allocator implementation.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <alloha/stack.h>

#include <alloha/core.h>
#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>

struct stack stack_new(usize capacity, u8* buf) {
    if (capacity != 0) {
        assert(buf && "stack_new called with inconsistent data: non-null capacity and null buffer");
    }

    return (struct stack){
        .buf             = buf,
        .capacity        = capacity,
        .offset          = 0,
        .previous_offset = 0,
    };
}

void stack_init(struct stack* restrict stack, usize capacity, u8* restrict buf) {
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

u8* stack_alloc_aligned(struct stack* stack, size_t size, u32 alignment) {
    if (!stack || stack->capacity == 0 || size == 0) {
        return NULL;
    }

    u8*   free_mem           = alloha_ptr_add(stack->buf, stack->offset);
    usize available_capacity = usize_wrap_sub(stack->capacity, stack->offset);

    u32 padding = padding_with_header(
        (uptr)free_mem,
        alignment,
        sizeof(struct stack_header),
        alloha_alignof(struct stack_header));
    usize required_size = (usize)padding + size;

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

    u8* new_block = alloha_ptr_add(free_mem, padding);

    // Write to the header associated with the new block of memory.
    struct stack_header* new_header =
        (struct stack_header*)alloha_ptr_sub(new_block, sizeof(struct stack_header));
    new_header->padding         = padding;
    new_header->capacity        = size;
    new_header->previous_offset = stack->previous_offset;

    // Update the stack offsets.
    stack->previous_offset = stack->offset + padding;
    stack->offset += required_size;

    return new_block;
}

u8* stack_alloc(struct stack* stack, usize size) {
    return stack_alloc_aligned(stack, size, ALLOHA_DEFAULT_ALIGNMENT);
}

bool stack_pop(struct stack* stack) {
    if (!stack || stack->offset == 0) {
        return false;
    }

    // Find info about the current top memory block.
    u8 const*                  top = alloha_ptr_add(stack->buf, stack->previous_offset);
    struct stack_header const* top_header =
        (struct stack_header const*)alloha_ptr_sub(top, sizeof(struct stack_header));

    // Update the stack.
    stack->offset          = stack->previous_offset - top_header->padding;
    stack->previous_offset = top_header->previous_offset;
    return true;
}

bool stack_clear_at(struct stack* restrict stack, u8* restrict block) {
    if (!stack || !block) {
        return false;
    }

    if (!(stack->buf <= block && block < alloha_ptr_add(stack->buf, stack->capacity))) {
        fprintf(
            stderr,
            "stack_free requested to free a block of memory outside of the range of the given "
            "stack allocator.\n");
        return false;
    }

    if (block >= alloha_ptr_add(stack->buf, stack->previous_offset)) {
        fprintf(stderr, "stack_free requested to free an already free memory block");
        return false;
    }

    struct stack_header const* block_header =
        (struct stack_header const*)alloha_ptr_sub(block, sizeof(struct stack_header));
    stack->offset =
        usize_wrap_sub(usize_wrap_sub((uptr)block, block_header->padding), (usize)stack->buf);
    stack->previous_offset = block_header->previous_offset;

    return true;
}

void stack_clear(struct stack* stack) {
    if (!stack) {
        return;
    }
    stack->offset          = 0;
    stack->previous_offset = 0;
}
