/**
 * @file Memory handling functions for megabuf/gmegabuf.
 * Provides functions to allocate, free and access memory buffers, both global and context-local ones.
 * See docs/Memory-Handling.md for details about thread safety, differences to ns-eel2 and other considerations.
 */
#pragma once

#include "CompilerTypes.h"

/**
 * @brief Destroys the global memory buffer and its contents.
 * Only to be used after all context objects are destroyed. Will cause segfaults otherwise.
 */
void prjm_eval_memory_destroy_global();

/**
 * @brief Returns the global memory buffer.
 * Can be used if only one global buffer is required. Call @a prjm_eval_memory_destroy_global()
 * to free this buffer.
 */
projectm_eval_mem_buffer prjm_eval_memory_global();

/**
 * @brief Creates a memory buffer which can hold the required amount of blocks.
 * @return A pointer to the empty buffer.
 */
projectm_eval_mem_buffer prjm_eval_memory_create_buffer();

/**
 * @brief Destroys a memory buffer and any blocks stored within.
 * @param buffer A pointer to the buffer which should be destroyed.
 */
void prjm_eval_memory_destroy_buffer(projectm_eval_mem_buffer buffer);

/**
 * @brief Frees the data stored in the buffer.
 * The buffer itself will not be destroyed. Call @a prjm_eval_memory_destroy_buffer() if this is needed.
 * @param buffer The buffer to clear.
 */
void prjm_eval_memory_free(projectm_eval_mem_buffer buffer);

/**
 * @brief Frees the data stored in a single block of the buffer.
 * The buffer itself will not be destroyed. Call @a prjm_eval_memory_destroy_buffer() if this is needed.
 * @param buffer The buffer to clear.
 * @param block The block to clear.
 */
void prjm_eval_memory_free_block(projectm_eval_mem_buffer buffer, int block);

/**
 * @brief Allocates the appropriate memory block and returns a pointer to the data address for the given index.
 * @param buffer A pointer to the buffer to use.
 * @param index The memory index (offset) to allocate and return a pointer to.
 * @return A pointer to the newly allocated value, or NULL if the allocation failed.
 */
PRJM_EVAL_F* prjm_eval_memory_allocate(projectm_eval_mem_buffer buffer, int index);

/**
 * @brief Copies a continuous block of values from one location to another.
 * @param buffer A pointer to the buffer to use.
 * @param offset_dest A pointer to a value with the memory start index to copy the values to.
 * @param offset_src A pointer to a value with the memory start index to copy the values from.
 * @param count A pointer to a value with the number of values to copy from src to dest.
 * @return Returns @a offset_dest.
 */
PRJM_EVAL_F* prjm_eval_memory_copy(projectm_eval_mem_buffer buffer,
                                   PRJM_EVAL_F* dest,
                                   PRJM_EVAL_F* src,
                                   PRJM_EVAL_F* len);

/**
 * @brief Sets a continuous block to a single value.
 * @param buffer A pointer to the buffer to use.
 * @param offset_dest The memory start index to set the values.
 * @param value The value to set.
 * @param count The number of values to set.
 *
 */
PRJM_EVAL_F* prjm_eval_memory_set(projectm_eval_mem_buffer buffer,
                                  PRJM_EVAL_F* dest,
                                  PRJM_EVAL_F* value,
                                  PRJM_EVAL_F* len);
