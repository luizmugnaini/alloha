/**
 * @brief Arena memory allocator implementation.
 * @file arena.c
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#include <alloha/arena.h>
#include <alloha/core.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void arena_init(arena_alloc_t* const arena, void* const buf, const size_t buf_size) {
    assert(buf);
    arena->buf = (unsigned char*)buf;
    arena->memory_owner = ALLOHA_FALSE;
    arena->capacity = buf_size;
    arena->offset = 0;
    arena->previous_offset = 0;
}

arena_alloc_t arena_create(size_t const capacity) {
    return (arena_alloc_t){
        .buf = malloc(capacity),
        .capacity = capacity,
        .offset = 0,
        .previous_offset = 0,
        .memory_owner = ALLOHA_TRUE,
    };
}

void* arena_alloc_aligned(arena_alloc_t* const arena, size_t const size, size_t const alignment) {
    uintptr_t const free_mem_ptr = (uintptr_t)arena->buf + (uintptr_t)arena->offset;
    uintptr_t const aligned_offset = align_forward(free_mem_ptr, alignment) - (uintptr_t)arena->buf;

    size_t required_mem = aligned_offset + size;
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
    arena_alloc_t* const arena,
    void* const old_mem,
    size_t const old_mem_size,
    size_t const new_size,
    size_t const alignment) {
    if (new_size == old_mem_size) {
        fprintf(stderr, "Irrelevant resizing of same size %zu.\n", old_mem_size);
        return old_mem;
    }
    if (!is_power_of_two(alignment)) {
        fprintf(
            stderr, "Expected the alignment to be a power of two, instead got %zu.\n", alignment);
        return 0;
    }
    if (!old_mem) {
        fprintf(
            stderr,
            "Expected a valid `old_mem` address. If you want to allocate new memory, use "
            "`arena_alloc_aligned` instead.\n");
        return 0;
    }

    unsigned char* const old_mem_bytes = (unsigned char*)old_mem;
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
    arena->offset = 0;
    arena->previous_offset = 0;
}

void arena_destroy(arena_alloc_t* arena) {
    if (arena->memory_owner && arena->buf) {
        free(arena->buf);
        *arena = (arena_alloc_t){0};
    }
}

temporary_arena_alloc_t temporary_arena_init(arena_alloc_t* const arena) {
    temporary_arena_alloc_t tmp_arena = {
        .arena = arena,
        .saved_offset = arena->offset,
        .saved_previous_offset = arena->previous_offset,
    };
    return tmp_arena;
}

void temporary_arena_end(temporary_arena_alloc_t* tmp_arena) {
    if (!tmp_arena) {
        fprintf(stderr, "temporary_arena_end called with an invalid temporary arena allocator.\n");
        return;
    }
    tmp_arena->arena->offset = tmp_arena->saved_offset;
    tmp_arena->arena->previous_offset = tmp_arena->saved_previous_offset;
    *tmp_arena = (temporary_arena_alloc_t){0};
}
