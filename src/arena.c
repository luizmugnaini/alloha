#include <alloha/arena.h>
#include <alloha/core.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void arena_init(ArenaAlloc* arena, const void* buf, const size_t buf_size) {
    if (buf) {
        arena->buf = (unsigned char*)buf;
        arena->mem_owner = ALLOHA_FALSE;
    } else {
        arena->buf = (unsigned char*)malloc(buf_size);
        arena->mem_owner = ALLOHA_TRUE;
    }
    arena->capacity = buf_size;
    arena->offset = 0;
    arena->previous_offset = 0;
}

uintptr_t align_forward(uintptr_t ptr, const size_t alignment) {
    if (!is_power_of_two(alignment)) {
        fprintf(stderr, "align_forward expected a power of two alignment, got %zu.\n", alignment);
        return 0;
    }

    uintptr_t al = (uintptr_t)alignment;
    uintptr_t mod =
        ptr & (al - 1);  // NOTE: Same as `ptr % al` (but faster) since `al` is a power of two.
    if (mod != 0) {
        // `ptr` is unaligned and we need to put it to the next aligned address.
        ptr += al - mod;
    }

    return ptr;
}

void* arena_alloc_aligned(ArenaAlloc* arena, const size_t size, const size_t alignment) {
    uintptr_t free_mem_ptr = (uintptr_t)arena->buf + (uintptr_t)arena->offset;
    uintptr_t aligned_offset = align_forward(free_mem_ptr, alignment) - (uintptr_t)arena->buf;

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
    void* mem_block = &arena->buf[aligned_offset];
    arena->previous_offset = aligned_offset;
    arena->offset = required_mem;
    return mem_block;
}

void* arena_alloc(ArenaAlloc* arena, const size_t size) {
    return arena_alloc_aligned(arena, size, DEFAULT_ALIGNMENT);
}

void* arena_resize(
    ArenaAlloc* arena,
    void* old_mem,
    const size_t old_mem_size,
    const size_t new_size,
    const size_t alignment) {
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

    unsigned char* old_mem_bytes = (unsigned char*)old_mem;
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

    void* new_mem = arena_alloc_aligned(arena, new_size, alignment);
    size_t min_copy_size = old_mem_size < new_size ? old_mem_size : new_size;
    memmove(new_mem, old_mem, min_copy_size);
    return new_mem;
}

void arena_free_all(ArenaAlloc* arena) {
    arena->offset = 0;
    arena->previous_offset = 0;
}

void arena_destroy(ArenaAlloc* arena) {
    if (arena->mem_owner) {
        free(arena->buf);
    }
}

TemporaryArenaAlloc temporary_arena_init(ArenaAlloc* arena) {
    TemporaryArenaAlloc tmp_arena;
    tmp_arena.arena = arena;
    tmp_arena.saved_offset = arena->offset;
    tmp_arena.saved_previous_offset = arena->previous_offset;
    return tmp_arena;
}

void temporary_arena_end(TemporaryArenaAlloc* tmp_arena) {
    if (!tmp_arena) {
        fprintf(stderr, "temporary_arena_end called with an invalid temporary arena allocator.\n");
        return;
    }
    tmp_arena->arena->offset = tmp_arena->saved_offset;
    tmp_arena->arena->previous_offset = tmp_arena->saved_previous_offset;
}
