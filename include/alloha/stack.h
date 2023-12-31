/**
 * @brief Stack memory allocator header file.
 * @file stack.h
 * @author Luiz G. Mugnaini Anselmo <luizmugnaini@gmail.com>
 */
#ifndef STACK_HEADER
#define STACK_HEADER

#include <stddef.h>  // for size_t

/**
 * Header associated with each memory block in the stack allocator.
 *
 * The header contains the following:
 * - `padding`: Information about the padding needed to contain itself and satisfy alignment
 *   properties;
 * - `previous_offset`: The offset to the last allocated memory memory block (just after its header)
 *   with respect to its associated memory block.
 *
 * The above can be summarized by the following diagram
 *
 *           `previous_offset`
 *                  ^
 *                  |
 *  |previous header|previous memory|+++++++|header|memory|
 *                                  ^              ^
 *                                  |--`padding`---|
 */
typedef struct {
    size_t padding; /**< Padding, in bytes, needed for the alignment of the memory block associated
                         with the header. The padding accounts for both the size of the header and
                         the needed alignment. */
    size_t previous_offset; /**< Pointer offset, relative to the stack allocator memory block, to
                                 the start of the memory address of the last allocated block (after
                                 its header). */
} StackAllocHeader;

/**
 * A stack memory allocator.
 *
 * The basic layout of the allocator looks like the following diagram:
 *
 *
 *          previous
 *           offset                          current
 *        for header 2                       offset
 *             ^                               ^
 *             |                               |
 *    |header 1|memory 1|++++|header 2|memory 2| free space |
 *    ^                 ^             ^                     ^
 *    |                 |---padding---|                     |
 *  start                             |                    end
 *    |                            previous                 |
 *    |                             offset                  |
 *    |                                                     |
 *    |--------------------- capacity ----------------------|
 *
 * Where each block of memory is preceded by a padding that comprises both the alignment needed for
 * the memory block and its corresponding header. This header will store the size of the padding and
 * the offset to the start the previous memory block with respect to itself.
 *
 * The allocator can be either the owner or merely the manager of the memory pointed by `buf`. If
 * the allocator is the owner, the user should always call `stack_destroy` in order to free
 * resources. If a block of memory is to be provided to the allocator for only management, the user
 * may instantiate the allocator via `stack_init` with a valid pointer to the available memory.
 */
typedef struct {
    unsigned char* buf; /**< Buffer holding the memory managed by the allocator. */
    size_t capacity;    /**< Capacity, in bytes, of the allocator. */
    size_t offset;      /**< Pointer offset relative to `buf` to the memory address where the free
                           space of `buf` starts. */
    size_t previous_offset; /**< Pointer offset relative to `buf` to the start of the memory
                                 address of the last allocated block (after its header). */
    int memory_owner;       /**< Whether the allocator owns the memory managed by the allocator. */
} StackAlloc;

// TODO: write docs

/**
 * Initializes a stack allocator that will take care of managing an externally allocated memory
 * block.
 *
 * @param stack Stack allocator to be initialized.
 * @param buf Buffer that the stack will manage for allocations.
 * @param buf_size Size of `buf` in bytes, which will become the capacity of `stack`.
 */
void stack_init(StackAlloc* const stack, void* const buf, size_t const buf_size);

/**
 * Creates a stack allocator that owns its memory.
 *
 * @param capacity Capacity, in bytes, that the stack allocator should have.
 * @return The resulting stack allocator.
 */
StackAlloc stack_create(size_t const capacity);

/**
 * Allocates a block of memory of a given size and alignment.
 *
 * @param stack Stack allocator that will contain and manage the new block of memory. Make sure the
 *        pointer to the stack is valid, otherwise you'll get a panic.
 * @param size Size, in bytes, of the new memory block.
 * @param alignment The needed alignment of the new memory block. This number should always be a
 *        power of two, otherwise the program will panic.
 *
 * @return If the allocation was successful, a pointer to the new block of memory is returned.
 *         If the stack is null or the alignment isn't a power of two, the program will panic. If
 *         the stack ran out of memory, a null pointer will be returned, so you should always check
 *         the returned pointer by this function.
 */
void* stack_alloc_aligned(StackAlloc* const stack, size_t const size, size_t const alignment);

/**
 * Allocates a block of memory of a given size using a default alignment.
 *
 * @param stack Stack allocator that will contain and manage the new block of memory. Make sure the
 *        pointer to the stack is valid, otherwise you'll get a panic.
 * @param size Size, in bytes, of the new memory block.
 *
 * @return If the allocation was successful, a pointer to the new block of memory is returned.
 *         If the stack is null the program will panic. If the stack ran out of memory, a null
 *         pointer will be returned, so you should always check the returned pointer by this
 *         function.
 */
void* stack_alloc(StackAlloc* const stack, size_t const size);

/**
 * Frees the last memory block allocated by the given stack.
 *
 * This function won't panic if the stack is empty, it will simply return.
 *
 * @param stack Pointer to the stack containing the memory block to be freed. If this pointer is
 *        null, the program will panic.
 */
void stack_pop(StackAlloc* const stack);

/**
 * Free all memory blocks up until the specified block pointed at by `ptr`.
 *
 * @param stack Pointer to the stack containing the blocks of memory to be freed. If this pointer is
 *        null, the program will panic.
 * @param ptr Pointer to the memory block that should be freed (all blocks above `ptr` will also be
 *        freed). If this pointer is null, outside of the stack allocator buffer, or already free,
 *        the program return false and won't panic.
 *
 * @return Returns the status of the operation: true if it was successful, false otherwise.
 */
int stack_free(StackAlloc* const stack, void* ptr);

/**
 * Free all allocated memory blocks of the stack.
 *
 * This resets both the `offset` and `previous_offset` of the stack. Notice that this won't destroy
 * the stack, just reset it.
 *
 * @param stack Pointer to the stack that should have all of its memory freed. If this pointer is
 *        null, the program will panic.
 */
void stack_free_all(StackAlloc* const stack);

/**
 * Destroy the memory managed by the stack
 */
void stack_destroy(StackAlloc* const stack);

#endif  // STACK_HEADER
