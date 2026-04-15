#include "TreeVariables.h"

#include "ctype.h"
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp _strnicmp
#endif

static PRJM_EVAL_F static_global_variables[100];

static prjm_eval_variable_entry_t* find_variable_entry(prjm_eval_compiler_context_t* cctx,
                                                       const char* name)
{
    /* Find existing variable */
    prjm_eval_variable_entry_t* var = cctx->variables.first;
    while (var)
    {
        if (strcasecmp(var->variable->name, name) == 0)
        {
            break;
        }
        var = var->next;
    }

    return var;
}

PRJM_EVAL_F* prjm_eval_register_variable(prjm_eval_compiler_context_t* cctx, const char* name)
{
    if (strlen(name) == 5 &&
        strncasecmp(name, "reg", 3) == 0 &&
        isdigit(name[3]) &&
        isdigit(name[4])
        )
    {
        int var_index = atoi(name + 3);
        if (var_index < 0 || var_index > 99)
        {
            var_index = 0;
        }

        if (cctx->global_variables == NULL)
        {
            cctx->global_variables = &static_global_variables;
        }

        return (*cctx->global_variables) + var_index;
    }

    prjm_eval_variable_entry_t* var = find_variable_entry(cctx, name);

    /* Create if it doesn't exist */
    if (!var)
    {
        var = malloc(sizeof(prjm_eval_variable_entry_t));
        var->variable = calloc(1, sizeof(prjm_eval_variable_def_t));
        var->variable->name = strdup(name);
        var->variable->value = .0f;
        var->next = cctx->variables.first;
        cctx->variables.first = var;
    }

    return &var->variable->value;
}
