#include "projectm-eval.h"

#include "projectm-eval/CompilerTypes.h"
#include "projectm-eval/MemoryBuffer.h"
#include "projectm-eval/CompileContext.h"
#include "projectm-eval/TreeVariables.h"

projectm_eval_mem_buffer projectm_eval_memory_buffer_create()
{
    return prjm_eval_memory_create_buffer();
}

void projectm_eval_memory_buffer_destroy(projectm_eval_mem_buffer buffer)
{
    prjm_eval_memory_destroy_buffer(buffer);
}

void projectm_eval_memory_global_destroy()
{
    prjm_eval_memory_destroy_global();
}

struct projectm_eval_context* projectm_eval_context_create(projectm_eval_mem_buffer global_mem,
                                                           PRJM_EVAL_F (* global_variables)[100])
{
    return prjm_eval_create_compile_context(global_mem, global_variables);
}

void projectm_eval_context_destroy(struct projectm_eval_context* ctx)
{
    prjm_eval_destroy_compile_context(ctx);
}

void projectm_eval_context_free_memory(struct projectm_eval_context* ctx)
{
    prjm_eval_memory_free(ctx->memory);
}

void projectm_eval_context_reset_variables(struct projectm_eval_context* ctx)
{
    prjm_eval_reset_context_vars(ctx);
}

PRJM_EVAL_F* projectm_eval_context_register_variable(struct projectm_eval_context* ctx, const char* var_name)
{
    return prjm_eval_register_variable(ctx, var_name);
}

struct projectm_eval_code* projectm_eval_code_compile(struct projectm_eval_context* ctx, const char* code)
{
    return (struct projectm_eval_code*) prjm_eval_compile_code(ctx, code);
}

void projectm_eval_code_destroy(struct projectm_eval_code* code_handle)
{
    prjm_eval_destroy_code((prjm_eval_program_t*) code_handle);
}

PRJM_EVAL_F projectm_eval_code_execute(struct projectm_eval_code* code_handle)
{
    if (!code_handle)
    {
        return 0.0;
    }

    PRJM_EVAL_F result = 0.0;
    PRJM_EVAL_F* result_ptr = &result;
    prjm_eval_program_t* eval_program = (prjm_eval_program_t*)code_handle;

    // Empty program.
    if (!eval_program->program)
    {
        return 0.0;
    }

    eval_program->program->func(eval_program->program, &result_ptr);

    return *result_ptr;
}

const char* projectm_eval_get_error(struct projectm_eval_context* ctx, int* line, int* column)
{
    if (line)
    {
        *line = ctx->error.line;
    }
    if (column)
    {
        *column = ctx->error.column_start;
    }
    return ctx->error.error;
}