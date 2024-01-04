/**
 * @brief Arena memory allocator implementation.
 * @file arena.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/arena.h>

#include <alloha/core.h>  // for DEFAULT_ALIGNMENT, ALLOHA_TRUE, ALLOHA_FALSE
#include <assert.h>       // for assert
#include <stdint.h>       // for uint8_t, uintptr_t
#include <stdio.h>        // for printf
#include <stdlib.h>       // for malloc, free
#include <string.h>       // for memmove

void arena_init(
    arena_alloc_t* const restrict arena, void* const restrict buf, const size_t buf_size) {
    assert(arena && "arena_init called with an invalid arena allocator");
    assert(buf && "arena_init called with an invalid memory block");
    arena->buf = (uint8_t*)buf;
    arena->capacity = buf_size;
    arena->offset = 0;
    arena->previous_offset = 0;
    arena->memory_owner = ALLOHA_FALSE;
}

arena_alloc_t arena_create(size_t const capacity) {
    assert(
        capacity != 0 &&
        "arena_create called with a requested capacity of zero, which isn't allowed");

    uint8_t* const buf = (uint8_t*)malloc(capacity);
    assert(buf && "arena_create couldn't allocate the requested amount of memory");

    return (arena_alloc_t){
        .buf = buf,
        .capacity = capacity,
        .offset = 0,
        .previous_offset = 0,
        .memory_owner = ALLOHA_TRUE,
    };
}

void* arena_alloc_aligned(arena_alloc_t* const arena, size_t const size, size_t const alignment) {
    assert(arena && "arena_alloc_aligned called with an invalid arena allocator");
    assert(
        size != 0 &&
        "arena_alloc_aligned requested to allocate zero bytes of memory, which is invalid");
    uintptr_t const free_mem_ptr = (uintptr_t)arena->buf + (uintptr_t)arena->offset;
    uintptr_t const aligned_offset = align_forward(free_mem_ptr, alignment) - (uintptr_t)arena->buf;

    size_t const required_mem = aligned_offset + size;
    if (required_mem > arena->capacity) {
        fprintf(
            stderr,
            "Unable to allocate %zu bytes of memory (%zu required), only %zu bytes left.\n",
            size,
            required_mem,
            arena->capacity - arena->offset);
        return 0;
    }

    void* const mem_block = &arena->buf[aligned_offset];
    arena->previous_offset = aligned_offset;
    arena->offset = required_mem;
    return mem_block;
}

void* arena_alloc(arena_alloc_t* const arena, size_t const size) {
    return arena_alloc_aligned(arena, size, DEFAULT_ALIGNMENT);
}

void* arena_resize(
    arena_alloc_t* const restrict arena,
    void* const restrict old_mem,
    size_t const old_mem_size,
    size_t const new_size,
    size_t const alignment) {
    assert(arena && "arena_resize called with an invalid arena allocator");
    assert(old_mem && "arena_resize called with an invalid block of memory");
    if (new_size == old_mem_size) {
        fprintf(stderr, "Irrelevant resizing of same size %zu.\n", old_mem_size);
        return old_mem;
    }
    if (!is_power_of_two(alignment)) {
        fprintf(
            stderr, "Expected the alignment to be a power of two, instead got %zu.\n", alignment);
        return 0;
    }

    uint8_t* const old_mem_bytes = (uint8_t*)old_mem;
    if (!(arena->buf <= old_mem_bytes && old_mem_bytes < arena->buf + arena->capacity)) {
        fprintf(
            stderr,
            "The given `old_mem` address lies outside of the buffer managed by the given arena "
            "allocator.\n");
        return 0;
    }

    // If the memory is the previously allocated, just adjust the offset.
    if (old_mem_bytes == arena->buf + arena->previous_offset) {
        arena->offset = arena->previous_offset + new_size;
        return old_mem;
    }

    void* const new_mem = arena_alloc_aligned(arena, new_size, alignment);
    size_t const min_copy_size = old_mem_size < new_size ? old_mem_size : new_size;
    memmove(new_mem, old_mem, min_copy_size);
    return new_mem;
}

void arena_free_all(arena_alloc_t* const arena) {
    assert(arena && "aren_free_all called with an invalid arena allocator");
    arena->offset = 0;
    arena->previous_offset = 0;
}

void arena_destroy(arena_alloc_t* const arena) {
    assert(arena && "arena_destroy called with an invalid arena allocator");
    if (arena->memory_owner && arena->buf) {
        free(arena->buf);
        *arena = (arena_alloc_t){0};
    }
}

temporary_arena_alloc_t temporary_arena_init(arena_alloc_t* const arena) {
    assert(arena && "temporary_arena_init called with an invalid arena allocator");
    return (temporary_arena_alloc_t){
        .arena = arena,
        .saved_offset = arena->offset,
        .saved_previous_offset = arena->previous_offset,
    };
}

void temporary_arena_end(temporary_arena_alloc_t* const tmp_arena) {
    assert(tmp_arena && "temporary_arena_end called with an invalid temporary arena allocator.\n");
    tmp_arena->arena->offset = tmp_arena->saved_offset;
    tmp_arena->arena->previous_offset = tmp_arena->saved_previous_offset;
    *tmp_arena = (temporary_arena_alloc_t){0};
}
