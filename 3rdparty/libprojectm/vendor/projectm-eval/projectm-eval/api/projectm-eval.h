/**
 * @file projectm_eval.h
 * @brief projectM Expression Evaluation Library Public API Header
 *
 * Defines all macros, stubs and methods to interface with the projectM Expression Evaluation Library.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Default floating-point number size in bytes (4 = float, 8 = double) */
#ifndef PRJM_F_SIZE
#define PRJM_F_SIZE 8
#endif

/* On 32-bit platforms, it may be more performant to use floats. */
#if PRJM_F_SIZE == 4
typedef float PRJM_EVAL_F;
#else
typedef double PRJM_EVAL_F;
#endif

/**
 * @brief Opaque context type which holds a variable/memory execution context.
 * The context stores all registered variables (internal and external) and the pointers
 * to the global and context-specific memory blocks used via megabuf and gmegabuf.
 */
struct projectm_eval_context;

/**
 * @brief Opaque type for compiled programs.
 * This code handle refers to a single compiled program. A program is always tied to a prjm_eval_context_t
 * and will become invalid once the context has been destroyed. A context can contain any number of code
 * handles, and code handles can be destroyed and recompiled at any time.
 */
struct projectm_eval_code;

/**
 * @brief Buffer pointer for megabuf/gmegabuf memory.
 */
typedef PRJM_EVAL_F** projectm_eval_mem_buffer;


/**
 * @brief Host-defined lock function.
 * Used to prevent race conditions with memory access. Only required if multiple expressions using the same
 * global/local memory blocks will run in separate threads at the same time. Can be an empty function otherwise.
 * The function is not required to use a recursive mutex.
 */
void projectm_eval_memory_host_lock_mutex();

/**
 * @brief Host-defined unlock function.
 * Used to prevent race conditions with memory access. Only required if multiple expressions using the same
 * global/local memory blocks will run in separate threads at the same time. Can be an empty function otherwise.
 */
void projectm_eval_memory_host_unlock_mutex();

/**
 * @brief Allocates an empty memory buffer to hold gmegabuf data.
 * @return A handle to a buffer which can be passed to @a projectm_eval_context_create().
 */
projectm_eval_mem_buffer projectm_eval_memory_buffer_create();

/**
 * @brief Destroys a memory buffer and frees any allocated blocks.
 * Only destroy a buffer if no context is using it anymore.
 * @param buffer A handle to the buffer which should be destroyed.
 */
void projectm_eval_memory_buffer_destroy(projectm_eval_mem_buffer buffer);

/**
 * @brief Frees the built-in global memory buffer.
 * Only destroy the global buffer if no context is using it anymore.
 */
void projectm_eval_memory_global_destroy();

/**
 * @brief Creates a new execution context.
 * @param global_mem An optional pointer to a prjm_eval_mem_buffer_t which will be used as the global
 *                   memory buffer (gmegabuf) or NULL to use the built-in global buffer.
 *                   This buffer must not be destroyed before he last context using it was destroyed.
 * @param global_variables An optional pointer to an array of 100 variables to be used as global storage for
 *                         the special reg00 to reg99 variables. If NULL, a built-in global storage will be used.
 * @return A handle to the new execution context, or NULL if the context could not be created.
 */
struct projectm_eval_context* projectm_eval_context_create(projectm_eval_mem_buffer global_mem,
                                                           PRJM_EVAL_F (* global_variables)[100]);

/**
 * @brief Destroys an execution context and frees all associated resources.
 * Any code and variable references associated with the destroyed context will become invalid
 * and must not be used after calling this function.
 * @note This will not destroy any remaining code handles. Call @a projectm_eval_code_destroy() on each handle separately.
 * @param ctx The context to destroy.
 */
void projectm_eval_context_destroy(struct projectm_eval_context* ctx);

/**
 * @brief Frees the allocated memory of the context-local buffer, effectively resetting it.
 * This will not clear the global memory buffer associated with the context.
 * @param ctx The context which memory should be freed.
 */
void projectm_eval_context_free_memory(struct projectm_eval_context* ctx);

/**
 * @brief Sets all context variables to 0.0.
 * Registered variables will be kept intact, as the pointers will not change. Global variables reg00 to reg99
 * will also stay unchanged.
 * @param ctx The context in which to reset the variables.
 */
void projectm_eval_context_reset_variables(struct projectm_eval_context* ctx);

/**
 * @brief Registers a variable and returns the value pointer.
 * Variables can be registered at any time. If the variable doesn't exist yet, it is created, otherwise
 * the existing variable is being returned. Any code compiled before or after will use the same variable,
 * referenced by its case-insensitive name.
 * @param ctx The context in which to register the variables.
 * @param var_name The name of the variable. Case-insensitive.
 * @return A pointer to the actual value of the variable.
 */
PRJM_EVAL_F* projectm_eval_context_register_variable(struct projectm_eval_context* ctx, const char* var_name);

/**
 * @brief Compiled the given code into an executable program.
 * Call @a projectm_eval_get_error() to retrieve the compiler error and location on compilation failure.
 * @param ctx The context to associate the code with.
 * @param code The code to compile.
 * @return A handle for the compiled program or NULL if compilation failed.
 */
struct projectm_eval_code* projectm_eval_code_compile(struct projectm_eval_context* ctx, const char* code);

/**
 * @brief Destroys a previously compiled code handle.
 * Frees only the compiled code, but no associated resources like variables and megabuf contents.
 * This makes it possible to recompile code and execute it in the same context without changing
 * the state. Do not use the code handle after destroying it.
 * @param code_handle The code handle to destroy.
 */
void projectm_eval_code_destroy(struct projectm_eval_code* code_handle);

/**
 * @brief Executes the code in the given handle.
 * @param code_handle The compiled code to execute.
 * @return The return value of the last expression on the top-level instruction list of the program.
 */
PRJM_EVAL_F projectm_eval_code_execute(struct projectm_eval_code* code_handle);

/**
 * @brief Returns the error message of the last failed compile operation in the given context.
 * The error message is cleared every time new code is compiled.
 * @param ctx The context to retrieve the error from.
 * @param line A pointer to an integer which will receive the line number in which the error was found.
 *             Pass NULL if the value is of no interest.
 * @param column A pointer to an integer which will receive the column number in which the error was found.
 *               Pass NULL if the value is of no interest.
 * @return A pointer to an error message. The context keeps ownership of the pointer, do not free it.
 */
const char* projectm_eval_get_error(struct projectm_eval_context* ctx, int* line, int* column);

#ifdef __cplusplus
};
#endif
