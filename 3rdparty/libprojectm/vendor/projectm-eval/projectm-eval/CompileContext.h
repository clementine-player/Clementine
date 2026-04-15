#pragma once

#include "CompilerTypes.h"

/**
 * @brief Creates an empty compile context.
 * @param global_memory An optional pointer to a memory buffer to use as global memory (gmegabuf).
 *                      If NULL, uses the built-in buffer.
 * @param global_variables An optional pointer to an array with 100 global variables.
 * @return A pointer to the newly created context.
 */
prjm_eval_compiler_context_t* prjm_eval_create_compile_context(projectm_eval_mem_buffer global_memory,
                                                               PRJM_EVAL_F (* global_variables)[100]);

/**
 * @brief Destroys a compile context.
 * Do not use the pointer afterwards.
 * @param cctx The context to destroy.
 */
void prjm_eval_destroy_compile_context(prjm_eval_compiler_context_t* cctx);

/**
 * @brief Compiles a program and returns a pointer to the result.
 * @param cctx The context to use for compilation.
 * @param code The code to compile.
 * @return A pointer to the resulting program tree or NULL on a parse error.
 */
prjm_eval_program_t* prjm_eval_compile_code(prjm_eval_compiler_context_t* cctx, const char* code);

/**
 * @brief Destroys a previously compiled program.
 * @param program The program to destroy.
 */
void prjm_eval_destroy_code(prjm_eval_program_t* program);

/**
 * @brief Resets all internal variable values to 0.
 * Externally registered variables are not changed.
 * @param cctx The compile context containing the variables.
 */
void prjm_eval_reset_context_vars(prjm_eval_compiler_context_t* cctx);

/**
 * @brief Returns the last error message.
 * @param cctx The context to retrieve the error from.
 * @param line A pointer to a variable that will receive the error's position line number or NULL.
 * @param column_start A pointer to a variable that will receive the error's first column number or NULL.
 * @param column_end A pointer to a variable that will receive the error's last column number or NULL.
 * @return A char pointer to the last error message or NULL if there wasn't an error. The pointer is owned by the context, do not free.
 */
const char* prjm_eval_compiler_get_error(prjm_eval_compiler_context_t* cctx,
                                         int* line,
                                         int* column_start,
                                         int* column_end);
