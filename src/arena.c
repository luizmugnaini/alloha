/// Arena memory allocator implementation.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#include <alloha/arena.h>

#include <alloha/core.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct arena arena_new(usize capacity, u8* buf) {
    if (capacity != 0) {
        assert(buf && "arena_new called with inconsistent data: non-null size but null buffer");
    }
    return (struct arena){
        .buf      = buf,
        .capacity = capacity,
        .offset   = 0,
    };
}

void arena_init(struct arena* restrict arena, usize capacity, u8* restrict buf) {
    if (!arena) {
        return;
    }
    if (capacity != 0) {
        assert(buf && "arena_init called with an inconsistent data: non-null size but null buffer");
    }

    arena->buf      = buf;
    arena->capacity = capacity;
    arena->offset   = 0;
}

u8* arena_alloc_aligned(struct arena* arena, usize size, u32 alignment) {
    if (!arena || arena->capacity == 0 || size == 0) {
        return NULL;
    }

    uptr const memory_addr    = (uptr)arena->buf;
    uptr const new_block_addr = align_forward(memory_addr + arena->offset, alignment);

    // Check if there is enough memory.
    if (new_block_addr + size > arena->capacity + memory_addr) {
        fprintf(
            stderr,
            "ArenaAlloc::alloc unable to allocate %zu bytes of memory (%zu bytes required due "
            "to alignment). The allocator has only %zu bytes remaining.\n",
            size,
            usize_wrap_sub(size + new_block_addr, (arena->offset + memory_addr)),
            arena->capacity - arena->offset);
        return NULL;
    }

    arena->offset = usize_wrap_sub(size + new_block_addr, memory_addr);
    return (u8*)new_block_addr;
}

u8* arena_alloc(struct arena* arena, usize size) {
    return arena_alloc_aligned(arena, size, ALLOHA_DEFAULT_ALIGNMENT);
}

u8* arena_realloc(
    struct arena* restrict arena,
    u8* restrict block,
    usize current_capacity,
    usize new_capacity,
    u32   alignment) {
    // Arena allocators cannot deallocate willy-nilly.
    assert(new_capacity != 0 && "ArenaAlloc::realloc called with a zero `new_capacity` parameter");

    // Check if there is any memory at all.
    if (!arena || arena->capacity == 0) {
        return NULL;
    }

    // Check if the user wants to allocate a completely new block.
    if (block == NULL || current_capacity == 0) {
        return arena_alloc_aligned(arena, new_capacity, alignment);
    }

    uptr const block_addr      = (uptr)block;
    uptr const memory_start    = (uptr)arena->buf;
    uptr const memory_end      = memory_start + arena->capacity;
    uptr const start_free_addr = memory_start + arena->offset;

    // Check if the block lies within the allocator's memory.
    if (block_addr < memory_start || block_addr >= memory_end) {
        fprintf(stderr, "arena_realloc called with pointer outside of its domain.\n");
        return NULL;
    }

    // Check if the block is already free.
    if (block_addr >= start_free_addr) {
        fprintf(
            stderr,
            "arena_realloc called with a pointer to a free address of the arena "
            "domain.\n");
        return NULL;
    }

    // If the block is the last allocated, just bump the offset.
    if (block_addr == usize_wrap_sub(start_free_addr, current_capacity)) {
        // Check if there is enough space.
        if (block_addr + new_capacity > memory_end) {
            fprintf(
                stderr,
                "arena_realloc unable to reallocate block from %zu bytes to %zu bytes.\n",
                current_capacity,
                new_capacity);
            return NULL;
        }

        arena->offset += usize_wrap_sub(new_capacity, current_capacity);
        return block;
    }

    u8* new_mem = arena_alloc_aligned(arena, new_capacity, alignment);
    if (!new_mem) {
        return NULL;
    }

    // Copy the existing data to the new block.
    usize copy_size = alloha_min(current_capacity, new_capacity);
    memory_copy(new_mem, block, copy_size);

    return new_mem;
}

void arena_clear(struct arena* arena) {
    if (!arena) {
        return;
    }
    arena->offset = 0;
}

struct scratch_arena scratch_arena_start(struct arena* arena) {
    assert(arena && "scratch_arena_start called with null arena");
    return (struct scratch_arena){
        .parent       = arena,
        .saved_offset = arena->offset,
    };
}

struct scratch_arena scratch_arena_decouple(struct scratch_arena const* scratch) {
    assert(
        (scratch && scratch->parent) && "scratch_arena_decouple called with a null scratch arena");
    return (struct scratch_arena){
        .parent       = scratch->parent,
        .saved_offset = scratch->parent->offset,
    };
}

void scratch_arena_end(struct scratch_arena* scratch) {
    if (!scratch || !scratch->parent) {
        return;
    }

    scratch->parent->offset = scratch->saved_offset;
    scratch->parent         = NULL;
    scratch->saved_offset   = 0;
}
