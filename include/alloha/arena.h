/**
 * @brief Arena allocator.
 * @file arena.h
 * @author Luiz G. Mugnaini A. <luizmugnaini@gmail.com>
 */
#ifndef ARENA_HEADER
#define ARENA_HEADER

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uintptr_t

#ifndef DEFAULT_ALIGNMENT
/** Alignment used in case the user doesn't provide one. */
#define DEFAULT_ALIGNMENT (2 * sizeof(void*))
#endif  // DEFAULT_ALIGNMENT

/** Arena allocator.
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
    unsigned char* buf; /**< Buffer containing the memory managed by the allocator. */
    size_t capacity;    /**< Capacity, in bytes, of `buf` (in this case, the length of `buf`). */
    size_t offset;      /**< Offset to the free memory space, relative to `buf`. */
    size_t previous_offset; /**< Offset to the start of the previously allocated memory region,
                                 relative to `buf`. */
    int memory_owner;       /**< Flag indicating if the allocator owns the memory. */
} arena_alloc_t;

/**
 * Initialize an arena allocator.
 *
 * @param arena A pointer to the arena allocator to be initialized.
 * @param buf Pointer to the block of memory that will be managed, but not owned, by the allocator.
 * @param buf_size Size, in bytes, of the provided block of memory `buf`.
 */
void arena_init(arena_alloc_t* const arena, void* const buf, size_t const buf_size);

/** Creates an arena allocator with a given capacity in bytes. */
arena_alloc_t arena_create(size_t const capacity);

/** Computes the next pointer with respect to `ptr` that has the required alignment. */
uintptr_t align_forward(uintptr_t ptr, size_t const alignment);

/** Allocates `size` bytes of memory with a given alignment. */
void* arena_alloc_aligned(arena_alloc_t* const arena, size_t const size, size_t const alignment);

/** Allocates `size` bytes of memory with the default alignment. */
void* arena_alloc(arena_alloc_t* const arena, size_t const size);

/**
 * Resizes a given block of memory that lies in the memory buffer managed by the arena allocator.
 *
 * @param arena A pointer to the arena allocator.
 * @param old_mem Pointer to the start of the memory block to be resized.
 * @param old_mem_size Size, in bytes, of `old_mem`.
 * @param new_size Desired size, in bytes, for the resizing of `old_mem`.
 * @param alignment The alignment to be used if a new allocation is needed.
 *
 * @return Pointer to the resized memory address. This function can return 0 if `old_mem` is an
 *         invalid address (null or lies outside of `arena`, or if the alignment is not a power of
 *         two.
 */
void* arena_resize(
    arena_alloc_t* const arena,
    void* const old_mem,
    size_t const old_mem_size,
    size_t const new_size,
    size_t const alignment);

/** Free all memory managed by the arena. */
void arena_free_all(arena_alloc_t* const arena);

/** Destroys the memory owned by the arena. */
void arena_destroy(arena_alloc_t* arena);

/**
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
 * Creates an instance of a `temporary_arena_alloc_t` with the current state of the offsets of
 * `arena`.
 */
temporary_arena_alloc_t temporary_arena_init(arena_alloc_t* const arena);

/**
 * Restores the offset state of the arena allocator saved in `tmp_arena` when `temporary_arena_init`
 * was called.
 */
void temporary_arena_end(temporary_arena_alloc_t* const tmp_arena);

#endif  // ARENA_HEADER
