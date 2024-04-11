/// Arena allocator.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#pragma once

#include <alloha/core.h>

/// Arena allocator
///
/// The arena allocator is great for the management of temporary allocation of memory, since an
/// allocation takes nothing more than incrementing an offset.
///
/// Memory layout:
///    |memory|padding|memory|  free space  |
///    ^              ^                     ^
///    |              |                     |
///  start        previous                 end
///    |           offset                   |
///    |                                    |
///    |------------ capacity --------------|
///
/// Note:
/// The arena **does not own** is memory, thus it **is not** responsible for the freeing of it. You
/// should never write code like this:
/// ```C
///  arena_t arena;
///  arena_init(&arena, 1024, malloc(1024));
///
///  ... or ...
///
///  arena_t arena = arena_new(2048, malloc(2048));
/// ```
/// since the block allocated by `malloc` will *never* be freed. Instead, you should write
/// something like this:
/// ```C
/// usize const mem_capacity = 1024;
/// u8* mem = (u8*)malloc(mem_capacity);
/// {
///     arena_t arena;
///     arena_init(arena, mem_capacity, mem);
///
///     ... or ...
///
///     arena_t arena = arena_new(mem_capacity, mem);
///
///     ... do stuff with your arena ...
/// }
/// free(mem);
/// ```
/// You should also check if `malloc` returned a valid pointer.
typedef struct arena_s {
    u8*   buf;       ///< Buffer containing the memory managed by the allocator.
    usize capacity;  ///< Capacity, in bytes, of `buf`.
    usize offset;    ///< Offset to the free memory space, relative to `buf`.
} arena_t;

/// Create a new arena.
arena_t arena_new(usize capacity, u8* buf);

/// Initialize an existing arena allocator.
///
/// Parameters:
///     * `arena`: A pointer to the arena allocator to be initialized.
///     * `capacity`: Size, in bytes, of the provided block of memory `buf`.
///     * `buf`: Pointer to the block of memory that will be managed, but not owned, by the
///              allocator.
void arena_init(arena_t* restrict arena, usize capacity, u8* restrict buf);

/// Allocate a block of memory satisfying a given alignment.
///
/// Parameters:
///     * `arena`: The arena allocator responsible for the allocation.
///     * `size`: The size, in bytes, of the new block of memory.
///     * `alignment`: The alignment, in bytes, needed by the new block of memory.
///
/// @return Pointer to the newly allocated block of memory. This can be null if the allocation
///         failed.
u8* arena_alloc_aligned(arena_t* arena, usize size, u32 alignment);

/// Allocates a block of memory with a default alignment.
///
/// Under the hood, calls `arena_alloc_aligned` with an alignment of `ALLOHA_DEFAULT_ALIGNMENT`.
///
/// Parameters:
///     * `arena`: The arena allocator responsible for the allocation.
///     * `size`: The size, in bytes, of the new block of memory.
u8* arena_alloc(arena_t* arena, usize size);

/// Reallocates a given block of memory.
///
/// Parameters:
///     * `arena`: A pointer to the arena allocator, whose memory should contain the block of memory
///     * `pointed`: by `old_mem`.
///     * `old_mem`: Pointer to the start of the memory block to be resized.
///     * `old_mem_size`: Size, in bytes, of `old_mem`.
///     * `new_size`: Desired size, in bytes, for the resizing of `old_mem`.
///     * `alignment`: The alignment to be used if a new allocation is needed. Should always be a
///                    power of two.
u8* arena_realloc(
    arena_t* restrict arena,
    u8* restrict old_mem,
    usize old_mem_size,
    usize new_size,
    u32   alignment);

/// Reset the arena's offset
void arena_clear(arena_t* arena);

/// Scratch arena allocator.
///
/// A temporary arena allocator has the purpose of saving the state of the current and previous
/// offsets of a given arena allocator. This allows for the user to quickly mess with the arena and
/// go back to the saved state.
///
/// Note: no copy is done, so changes to the memory will be persistent.
typedef struct scratch_arena_s {
    arena_t* parent;
    usize    saved_offset;
} scratch_arena_t;

/// Create a new scratch arena allocator.
scratch_arena_t scratch_arena_start(arena_t* arena);

/// Create a new scratch arena with the current state of the parent of `scratch`.
scratch_arena_t scratch_arena_decouple(scratch_arena_t const* scratch);

/// Restore the state of the associated arena allocator.
///
/// Restores the offset state of the arena allocator saved in `scratch` when `scratch_arena_start`
/// was called.
void scratch_arena_end(scratch_arena_t* scratch);
