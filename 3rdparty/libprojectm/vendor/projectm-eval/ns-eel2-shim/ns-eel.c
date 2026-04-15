#include "ns-eel.h"

#include "projectm-eval/api/projectm-eval.h"
#include "projectm-eval/CompilerTypes.h"
#include "projectm-eval/CompileContext.h"
#include "projectm-eval/TreeVariables.h"

void projectm_eval_memory_host_lock_mutex()
{
    NSEEL_HOSTSTUB_EnterMutex();
}

void projectm_eval_memory_host_unlock_mutex()
{
    NSEEL_HOSTSTUB_LeaveMutex();
}

void NSEEL_quit()
{
    projectm_eval_memory_global_destroy();
}

NSEEL_VMCTX NSEEL_VM_alloc()
{
    return (NSEEL_VMCTX) projectm_eval_context_create(NULL, NULL);
}

void NSEEL_VM_free(NSEEL_VMCTX ctx)
{
    projectm_eval_context_destroy((struct projectm_eval_context*) ctx);
}

void NSEEL_VM_resetvars(NSEEL_VMCTX ctx)
{
    projectm_eval_context_reset_variables((struct projectm_eval_context*) ctx);
}

EEL_F *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name)
{
    return prjm_eval_register_variable((struct projectm_eval_context*) ctx, name);
}

void NSEEL_VM_freeRAM(NSEEL_VMCTX ctx)
{
    projectm_eval_context_free_memory((struct projectm_eval_context*) ctx);
}

void NSEEL_VM_SetGRAM(NSEEL_VMCTX ctx, void **gram)
{
    struct projectm_eval_context* prjm_ctx = (struct projectm_eval_context*)ctx;
    prjm_ctx->global_memory = (projectm_eval_mem_buffer)gram;
}

void NSEEL_VM_FreeGRAM(void **ufd)
{
    projectm_eval_memory_buffer_destroy((projectm_eval_mem_buffer) ufd);
}

NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, char *code, int lineoffs)
{
    // ToDo: Forward passed pointer to skip "lineoffs" \n chars for compatibility
    return (NSEEL_CODEHANDLE) projectm_eval_code_compile((struct projectm_eval_context*) ctx, code);
}

char *NSEEL_code_getcodeerror(NSEEL_VMCTX ctx)
{
    return ((struct projectm_eval_context*)ctx)->error.error;
}

void NSEEL_code_execute(NSEEL_CODEHANDLE code)
{
    projectm_eval_code_execute((struct projectm_eval_code*) code);
}

void NSEEL_code_free(NSEEL_CODEHANDLE code)
{
    projectm_eval_code_destroy((struct projectm_eval_code*) code);
}
