/**
 * @brief Stack memory allocator implementation.
 * @file stack.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/core.h>  // for is_power_of_two, DEFAULT_ALIGNMENT, ALLOHA_FALSE, ALLOHA_TRUE
#include <alloha/stack.h>

#include <assert.h>  // for assert
#include <stddef.h>  // for size_t
#include <stdint.h>  // for uintptr_t
#include <stdio.h>   // for fprintf, stderr
#include <stdlib.h>  // for malloc, free

static size_t const k_stack_header_size = sizeof(StackAllocHeader);

void stack_init(StackAlloc* const stack, void* const buf, size_t const buf_size) {
    assert(stack && "stack_init called with a invalid stack allocator.");
    assert(buf && "stack_init called with an invalid memory buffer.");
    stack->buf = (unsigned char*)buf;
    stack->capacity = buf_size;
    stack->offset = 0;
    stack->previous_offset = 0;
    stack->memory_owner = ALLOHA_FALSE;
}

StackAlloc stack_create(size_t const capacity) {
    assert(capacity != 0 && "stack_create called with zero capacity.");
    return (StackAlloc){
        .buf = malloc(capacity),
        .capacity = capacity,
        .offset = 0,
        .memory_owner = ALLOHA_TRUE,
    };
}

size_t padding_with_header(uintptr_t const ptr, size_t const alignment, size_t const header_size) {
    assert(
        is_power_of_two(alignment) &&
        "padding_with_header expected the alignment to be a power of two");
    uintptr_t padding = 0;
    uintptr_t const al = (uintptr_t)alignment;
    uintptr_t const mod = ptr & (al - 1);
    if (mod != 0) {
        padding = al - mod;
    }
    printf("Padding without header: %u\n", (unsigned)padding);

    uintptr_t required_mem_size = (uintptr_t)header_size;
    if (padding < required_mem_size) {
        required_mem_size -= padding;
        if ((required_mem_size & (al - 1)) != 0) {
            padding += al * (1 + (required_mem_size / al));
        } else {
            padding += al * (required_mem_size / al);
        }
    }
    return (size_t)padding;
}

void* stack_alloc_aligned(StackAlloc* const stack, size_t const size, size_t const alignment) {
    assert(stack && "stack_alloc_aligned called with an invalid stack allocator");
    assert(size != 0 && "stack_alloc_aligned called with a request for a zero sized allocation");

    uintptr_t const current_addr = (uintptr_t)stack->buf + (uintptr_t)stack->offset;
    size_t const padding = padding_with_header(current_addr, alignment, k_stack_header_size);
    printf("Padding with header: %zu\n", padding);
    if (padding + size > stack->capacity - stack->offset) {
        fprintf(
            stderr,
            "Unable to allocate %zu bytes of memory (%zu bytes required), only %zu "
            "remaining.\n",
            size,
            padding + size,
            stack->capacity - stack->offset);
        return 0;
    }

    // Get the address pointer and write its header to memory.
    uintptr_t const requested_addr = current_addr + (uintptr_t)padding;
    StackAllocHeader* const requested_addr_header =
        (StackAllocHeader*)(requested_addr - (uintptr_t)k_stack_header_size);
    requested_addr_header->padding = padding;
    requested_addr_header->previous_offset = stack->previous_offset;

    stack->previous_offset = stack->offset + padding;  // offset to `requested_addr`
    stack->offset += padding + size;

    return (void*)requested_addr;
}

void* stack_alloc(StackAlloc* const stack, size_t const size) {
    return stack_alloc_aligned(stack, size, DEFAULT_ALIGNMENT);
}

void stack_pop(StackAlloc* const stack) {
    assert(stack && "stack_pop called with an invalid stack allocator.");
    if (stack->previous_offset == 0) {
        return;
    }

    uintptr_t const last_addr = (uintptr_t)stack->buf + (uintptr_t)stack->previous_offset;
    StackAllocHeader const* const last_addr_header =
        (StackAllocHeader const*)(last_addr - (uintptr_t)k_stack_header_size);
    stack->offset = stack->previous_offset - last_addr_header->padding;
    stack->previous_offset = last_addr_header->previous_offset;
}

int stack_free(StackAlloc* const stack, void* ptr) {
    assert(stack && "stack_free called with an invalid stack allocator.");
    if (!ptr) {
        fprintf(stderr, "stack_free requested to free a null pointer.\n");
        return ALLOHA_FALSE;
    }

    uintptr_t const addr = (uintptr_t)ptr;
    uintptr_t const start_addr = (uintptr_t)stack->buf;
    uintptr_t const end_addr = start_addr + (uintptr_t)stack->capacity;
    if (!(start_addr <= addr && addr < end_addr)) {
        fprintf(
            stderr,
            "stack_free requested to free a block of memory outside of the range of the given "
            "stack allocator.\n");
        return ALLOHA_FALSE;
    }
    if (addr >= start_addr + (uintptr_t)stack->offset) {
        fprintf(stderr, "stack_free requested to free an already free memory block");
        return ALLOHA_FALSE;
    }

    StackAllocHeader const* const header_addr =
        (StackAllocHeader const*)(addr - (uintptr_t)k_stack_header_size);
    stack->offset = (size_t)addr - header_addr->padding - (size_t)start_addr;
    stack->previous_offset = header_addr->previous_offset;

    return ALLOHA_TRUE;
}

void stack_free_all(StackAlloc* const stack) {
    assert(stack && "stack_free_all called with an invalid stack allocator.");
    stack->offset = 0;
    stack->previous_offset = 0;
}

void stack_destroy(StackAlloc* const stack) {
    assert(stack && "stack_free_all called with an invalid stack allocator.");
    if (stack->memory_owner && stack->buf) {
        free(stack->buf);
        *stack = (StackAlloc){0};
    }
}
