/**
 * @brief Stack memory allocator implementation.
 * @file stack.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/stack.h>

#include <alloha/core.h>  // for is_power_of_two, ALLOHA_DEFAULT_ALIGNMENT, ALLOHA_FALSE, ALLOHA_TRUE
#include <assert.h>       // for assert
#include <stddef.h>       // for size_t
#include <stdint.h>       // for uintptr_t, uint8_t
#include <stdio.h>        // for fprintf, stderr
#include <stdlib.h>       // for malloc, free

static size_t const k_stack_header_size = sizeof(stack_alloc_header_t);
static size_t const k_stack_header_alignment = sizeof(size_t);

void stack_init(
    stack_alloc_t* const restrict stack, void* const restrict buf, size_t const buf_size) {
    assert(stack && "stack_init called with a invalid stack allocator");
    assert(buf && "stack_init called with an invalid memory buffer");
    stack->buf = (uint8_t*)buf;
    stack->capacity = buf_size;
    stack->offset = 0;
    stack->previous_offset = 0;
    stack->memory_owner = ALLOHA_FALSE;
}

stack_alloc_t stack_create(size_t const capacity) {
    assert(capacity != 0 && "stack_create called with zero capacity");
    uint8_t* const buf = (uint8_t*)malloc(capacity);
    assert(buf && "Unable to allocate memory for the stack allocator");
    return (stack_alloc_t){
        .buf = buf,
        .capacity = capacity,
        .offset = 0,
        .memory_owner = ALLOHA_TRUE,
    };
}

void* stack_alloc_aligned(stack_alloc_t* const stack, size_t const size, size_t const alignment) {
    assert(stack && "stack_alloc_aligned called with an invalid stack allocator");
    assert(size != 0 && "stack_alloc_aligned called with a request for a zero sized allocation");

    uintptr_t const current_addr = (uintptr_t)stack->buf + (uintptr_t)stack->offset;
    size_t const padding =
        padding_with_header(current_addr, alignment, k_stack_header_size, k_stack_header_alignment);
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
    stack_alloc_header_t* const requested_addr_header =
        (stack_alloc_header_t*)(requested_addr - (uintptr_t)k_stack_header_size);
    requested_addr_header->padding = padding;
    requested_addr_header->previous_offset = stack->previous_offset;

    stack->previous_offset = stack->offset + padding;  // offset to `requested_addr`
    stack->offset += padding + size;

    return (void*)requested_addr;
}

void* stack_alloc(stack_alloc_t* const stack, size_t const size) {
    return stack_alloc_aligned(stack, size, ALLOHA_DEFAULT_ALIGNMENT);
}

int stack_pop(stack_alloc_t* const stack) {
    assert(stack && "stack_pop called with an invalid stack allocator");
    if (stack->offset == 0) {
        return ALLOHA_FALSE;
    }

    uintptr_t const last_addr = (uintptr_t)stack->buf + (uintptr_t)stack->previous_offset;
    stack_alloc_header_t const* const last_addr_header =
        (stack_alloc_header_t const*)(last_addr - (uintptr_t)k_stack_header_size);
    stack->offset = stack->previous_offset - last_addr_header->padding;
    stack->previous_offset = last_addr_header->previous_offset;
    return ALLOHA_TRUE;
}

int stack_free(stack_alloc_t* const restrict stack, void* restrict ptr) {
    assert(stack && "stack_free called with an invalid stack allocator");
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
    if (addr >= start_addr + (uintptr_t)stack->previous_offset) {
        fprintf(stderr, "stack_free requested to free an already free memory block");
        return ALLOHA_FALSE;
    }

    stack_alloc_header_t const* const header_addr =
        (stack_alloc_header_t const*)(addr - (uintptr_t)k_stack_header_size);
    stack->offset = (size_t)addr - header_addr->padding - (size_t)start_addr;
    stack->previous_offset = header_addr->previous_offset;

    return ALLOHA_TRUE;
}

void stack_free_all(stack_alloc_t* const stack) {
    assert(stack && "stack_free_all called with an invalid stack allocator");
    stack->offset = 0;
    stack->previous_offset = 0;
}

void stack_destroy(stack_alloc_t* const stack) {
    assert(stack && "stack_free_all called with an invalid stack allocator");
    if (stack->memory_owner && stack->buf) {
        free(stack->buf);
        *stack = (stack_alloc_t){0};
    }
}
