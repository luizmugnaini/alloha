/// Stack memory allocator header file.
///
/// Author: Luiz G. Mugnaini A. <luizmugnaini@gmail.com>

#pragma once

#include <alloha/core.h>

/// Header associated with each memory block in the stack allocator.
///
/// Memory layout:
///
/// ```md
///           `previous_offset`                     |-`capacity`-|
///                  ^                              ^            ^
///                  |                              |            |
///  |previous header|previous memory|+++++++|header|  memory    |
///                                  ^              ^
///                                  |--`padding`---|
/// ```
///
/// where "header" represents this current header, and "memory" represents the memory block
/// associated to this header.
struct stack_header {
    /// Padding, in bytes, needed for the alignment of the memory block associated with the header.
    /// The padding accounts for both the size of the header and the needed alignment.
    usize padding;

    /// The capacity, in bytes, of the memory block associated with this header.
    usize capacity;

    /// Pointer offset, relative to the stack allocator memory  block, to the start of the memory
    /// address of the last allocated block (after its header).
    usize previous_offset;
};

/// Stack memory allocator.
///
/// Note: The stack allocator **doesn't own** its memory, but merely manages it. That being said,
///       a stack allocator will never call `malloc` or `free`
///
/// Memory layout:
///
/// ```md
///          previous
///           offset                          current
///        for header 2                       offset
///             ^                               ^
///             |                               |
///    |header 1|memory 1|++++|header 2|memory 2| free space |
///    ^                 ^             ^                     ^
///    |                 |---padding---|                     |
///  start                             |                    end
///    |                            previous                 |
///    |                             offset                  |
///    |                                                     |
///    |--------------------- capacity ----------------------|
/// ```
///
/// Where each block of memory is preceded by a padding that comprises both the alignment needed
/// for the memory block and its corresponding header. This header will store the size of the
/// padding and the offset to the start the previous memory block with respect to itself.
///
/// The allocator can be either the owner or merely the manager of the memory pointed by
/// `memory`. If the allocator is the owner, the user should always call `stack_destroy` in order
/// to free resources. If a block of memory is to be provided to the allocator for only
/// management, the user may instantiate the allocator via `stack_init` with a valid pointer to
/// the available memory.
///
/// It is to be noted that the pointers returned by the functions associated to `stack_t` are
/// all raw pointers. This means that if you get a memory block via `stack_alloc` and later
/// free it via a `stack_clear_at`, you'll end up with a dangling pointer and
/// use-after-free problems may arise if you later read from this pointer. This goes to say that
/// the user should know how to correctly handle their memory reads and writes.
struct stack {
    /// Pointer to the memory region managed by the allocator.
    u8* buf;

    /// Total capacity, in bytes, of the allocator.
    usize capacity;

    /// Offset, in bytes, to the start of the free space region.
    usize offset;

    /// Pointer offset relative to the start of the memory address of the last allocated block
    /// (after its header).
    usize previous_offset;
};

/// Create a new stack allocator.
///
/// Parameters:
///     * `capacity`: Size of `buf` in bytes, which will become the capacity of `stack`.
///     * `buf`: Buffer that the stack will manage for allocations.
struct stack stack_new(usize capacity, u8* buf);

/// Initialize an existing stack allocator.
///
/// Initializes a stack allocator that will take care of managing an externally allocated memory
/// block.
///
/// Parameters:
///     * `stack`: Stack allocator to be initialized.
///     * `capacity`: Size of `buf` in bytes, which will become the capacity of `stack`.
///     * `buf`: Buffer that the stack will manage for allocations.
void stack_init(struct stack* restrict stack, usize capacity, u8* restrict buf);

/// Allocate a block of memory satisfying a given alignment.
///
/// Parameters:
///     * `stack`: Stack allocator that will contain and manage the new block of memory. Make sure
///                the pointer to the stack is valid, otherwise you'll get a panic.
///     * `size`: Size, in bytes, of the new memory block.
///     * `alignment`: The needed alignment of the new memory block. This number should always be a
///                    power of two, otherwise the program will panic.
u8* stack_alloc_aligned(struct stack* stack, usize size, u32 alignment);

/// Allocate a block of memory satisfying a default alignment.
///
/// Under the hood, calls `stack_alloc_aligned` with an alignment of `ALLOHA_DEFAULT_ALIGNMENT`.
///
/// Parameters:
///     * `stack`: Stack allocator that will contain and manage the new block of memory. Make sure
///                the pointer to the stack is valid, otherwise you'll get a panic.
///     * `size`: Size, in bytes, of the new memory block.
u8* stack_alloc(struct stack* stack, usize size);

/// Clear the last memory block allocated by the given stack.
///
/// This function won't panic if the stack is empty, it will simply return `ALLOHA_FALSE`.
///
/// Parameters:
///     * `stack`: Pointer to the stack containing the memory block to be freed. If this pointer is
///                null, the program will panic.
///
/// Return: state of the operation.
bool stack_pop(struct stack* stack);

/// Clear all memory blocks up until the specified memory block.
///
/// The stack allocator won't destroy its corresponding memory, it will simply restore the offsets
/// accordingly and the memory will be available for consequent allocations.
///
/// Parameters:
///     * `stack`: Pointer to the stack containing the blocks of memory to be freed. If this pointer
///                is null, the program will panic.
///     * `block`: Pointer to the memory block that should be freed (all blocks above `ptr` will
///                also be freed). If this pointer is null, outside of the stack allocator buffer,
///                or already free, the program return false and won't panic.
///
/// Return: status of the operation.
bool stack_clear_at(struct stack* restrict stack, u8* restrict block);

/// Clear all allocated memory blocks of the stack.
///
/// This resets all offsets of the stack. Notice that this won't destroy the stack, just reset its
/// original state, hence the associated memory will still be available for consequent allocations.
///
/// Parameters:
///     * `stack`: Pointer to the stack that should have all of its memory freed. If this pointer is
///               null, the program will panic.
void stack_clear(struct stack* stack);
