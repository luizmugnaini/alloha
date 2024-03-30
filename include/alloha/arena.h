/**
 * @brief Arena allocator.
 * @file arena.h
 * @author Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
 */
#ifndef ARENA_HEADER
#define ARENA_HEADER

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uintptr_t, uint8_t

/**
 * @brief Arena memory allocator.
 *
 * The allocator memory layout looks like the following diagram:
 *
 *    |memory|padding|memory|  free space  |
 *    ^              ^                     ^
 *    |              |                     |
 *  start        previous                 end
 *    |           offset                   |
 *    |                                    |
 *    |------------ capacity --------------|
 */
typedef struct arena_alloc_s {
    uint8_t* buf;    /**< Buffer containing the memory managed by the allocator. */
    size_t capacity; /**< Capacity, in bytes, of `buf` (in this case, the length of `buf`). */
    size_t offset;   /**< Offset to the free memory space, relative to `buf`. */
    size_t previous_offset; /**< Offset to the start of the previously allocated memory region,
                                 relative to `buf`. */
    int memory_owner;       /**< Flag indicating if the allocator owns the memory. */
} arena_alloc_t;

/**
 * @brief Initialize an existing arena allocator.
 *
 * @param arena A pointer to the arena allocator to be initialized.
 * @param buf Pointer to the block of memory that will be managed, but not owned, by the allocator.
 * @param buf_size Size, in bytes, of the provided block of memory `buf`.
 */
void arena_init(
    arena_alloc_t* const restrict arena, void* const restrict buf, size_t const buf_size);

/**
 * @brief Create a new arena allocator.
 *
 * @param capacity Capacity, in bytes, of the new allocator.
 *
 * @return The resulting arena allocator.
 */
arena_alloc_t arena_create(size_t const capacity);

/**
 * @brief Allocate a block of memory satisfying a given alignment.
 *
 * @param arena The arena allocator responsible for the allocation.
 * @param size The size, in bytes, of the new block of memory.
 * @param alignment The alignment, in bytes, needed by the new block of memory.
 *
 * @return Pointer to the newly allocated block of memory. This can be null if the allocation
 *         failed.
 */
void* arena_alloc_aligned(arena_alloc_t* const arena, size_t const size, size_t const alignment);

/**
 * @brief Allocates a block of memory with a default alignment.
 *
 * Under the hood, calls `arena_alloc_aligned` with an alignment of `ALLOHA_DEFAULT_ALIGNMENT`.
 *
 * @param arena The arena allocator responsible for the allocation.
 * @param size The size, in bytes, of the new block of memory.
 *
 * @return Pointer to the newly allocated block of memory. This can be null if the allocation
 *         failed.
 */
void* arena_alloc(arena_alloc_t* const arena, size_t const size);

/**
 * @brief Resizes a given block of memory.
 *
 * @param arena A pointer to the arena allocator, whose memory should contain the block of memory
 *        pointed by `old_mem`.
 * @param old_mem Pointer to the start of the memory block to be resized.
 * @param old_mem_size Size, in bytes, of `old_mem`.
 * @param new_size Desired size, in bytes, for the resizing of `old_mem`.
 * @param alignment The alignment to be used if a new allocation is needed.
 *
 * @return Pointer to the resized memory address. This function can return null if `old_mem` is an
 *         invalid address (null or lies outside of `arena`, or if the alignment is not a power of
 *         two.
 */
void* arena_resize(
    arena_alloc_t* const restrict arena,
    void* const restrict old_mem,
    size_t const old_mem_size,
    size_t const new_size,
    size_t const alignment);

/**
 * @brief Free all memory managed by the arena.
 *
 * This function does not free any actual memory, it only restores the offsets of the arena to their
 * original zeroed state. Thus allocated blocks will be simply overwritten in any new allocation.
 *
 * @param arena The arena allocator that should have its memory freed.
 */
void arena_clear(arena_alloc_t* const arena);

/** Destroys the memory owned by the arena. */
void arena_destroy(arena_alloc_t* const arena);

/**
 * @brief Temporary arena memory allocator.
 *
 * A temporary arena allocator has the purpose of saving the state of the current and previous
 * offsets of a given arena allocator. This allows for the user to quickly mess with the arena and
 * go back to the saved state.
 *
 * **Notice that no copy is done, so changes to the memory will be persistent.**
 */
typedef struct temporary_arena_alloc_s {
    arena_alloc_t* arena; /**< Arena allocator to be used by the temporary allocator. */
    size_t saved_offset;  /**< The offset of `arena` when the temporary allocator was created. */
    size_t saved_previous_offset; /**< The previous offset allocated by `arena` when the temporary
                                       allocator was created. */
} temporary_arena_alloc_t;

/**
 * @brief Create a new temporary arena allocator.
 *
 * @param arena The arena allocator with the state that should be saved to the temporary arena
 *        allocator.
 *
 * @return The resulting temporary arena allocator.
 */
temporary_arena_alloc_t temporary_arena_init(arena_alloc_t* const arena);

/**
 * @brief Restore the state of the associated arena allocator.
 *
 * Restores the offset state of the arena allocator saved in `tmp_arena` when `temporary_arena_init`
 * was called.
 *
 * @param tmp_arena The temporary arena allocator containing the saved offset states and the arena
 *        allocator that should have its state restored.
 */
void temporary_arena_end(temporary_arena_alloc_t* const tmp_arena);

#endif  // ARENA_HEADER
