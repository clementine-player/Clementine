#include "CompilerFunctions.h"

#include "ExpressionTree.h"
#include "TreeFunctions.h"
#include "TreeVariables.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp _strnicmp
#endif

/* Called by yyparse on error. */
void prjm_eval_error(PRJM_EVAL_LTYPE* loc, prjm_eval_compiler_context_t* cctx, yyscan_t yyscanner, char const* s)
{
    cctx->error.error = strdup(s);
    cctx->error.line = loc->first_line;
    cctx->error.column_start = loc->first_column;
    cctx->error.column_end = loc->last_column;
}

void prjm_eval_compiler_destroy_arglist(prjm_eval_compiler_arg_list_t* arglist)
{
    if (!arglist)
    {
        return;
    }

    prjm_eval_compiler_arg_node_t* arg = arglist->begin;
    while (arg)
    {
        prjm_eval_compiler_arg_node_t* free_arg = arg;
        arg = arg->next;
        if (free_arg->node)
        {
            prjm_eval_compiler_destroy_node(free_arg->node);
        }
        free(free_arg);
    }

    free(arglist);
}

void prjm_eval_compiler_destroy_node(prjm_eval_compiler_node_t* node)
{
    if (node->tree_node)
    {
        prjm_eval_destroy_exptreenode(node->tree_node);
    }
    free(node);
}

bool prjm_eval_compiler_name_is_function(prjm_eval_compiler_context_t* cctx, const char* name)
{
    prjm_eval_function_list_item_t* entry = cctx->functions.first;
    while (entry)
    {
        if (strcasecmp(entry->function->name, name) == 0)
        {
            return true;
        }

        entry = entry->next;
    }

    return false;
}

prjm_eval_function_def_t* prjm_eval_compiler_get_function(prjm_eval_compiler_context_t* cctx, const char* name)
{
    prjm_eval_function_list_item_t* func = cctx->functions.first;
    while (func)
    {
        if (strcasecmp(func->function->name, name) == 0)
        {
            return func->function;
        }

        func = func->next;
    }

    return NULL;
}

prjm_eval_compiler_arg_list_t* prjm_eval_compiler_add_argument(prjm_eval_compiler_arg_list_t* arglist,
                                                               prjm_eval_compiler_node_t* arg)
{
    prjm_eval_compiler_arg_node_t* arg_node = calloc(1, sizeof(prjm_eval_compiler_arg_node_t));

    if (arg_node == NULL)
    {
        return NULL;
    }

    arg_node->node = arg;

    if (!arglist)
    {
        arglist = calloc(1, sizeof(prjm_eval_compiler_arg_list_t));
        if (!arglist)
        {
            free(arg_node);
            return NULL;
        }
        arglist->begin = arg_node;
    }
    else
    {
        arglist->end->next = arg_node;
    }

    arglist->end = arg_node;
    arglist->count++;

    return arglist;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_create_function(prjm_eval_compiler_context_t* cctx,
                                                             const char* name,
                                                             prjm_eval_compiler_arg_list_t* arglist,
                                                             char** error)
{
    prjm_eval_function_def_t* func = prjm_eval_compiler_get_function(cctx, name);
    if (!func)
    {
        PRJM_EVAL_FORMAT_ERROR(*error, "Unknown function \"%s\".", name)
        return NULL;
    }

    if (func->arg_count != arglist->count)
    {
        PRJM_EVAL_FORMAT_ERROR(*error, "Invalid argument count for function \"%s\": Expected %d, but %d given.",
                              name, func->arg_count, arglist->count)
        return NULL;
    }

    prjm_eval_compiler_node_t* node = prjm_eval_compiler_create_expression(cctx, func, arglist);

    return node;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_create_expression_empty(prjm_eval_function_def_t* func)
{
    prjm_eval_exptreenode_t* expr = calloc(1, sizeof(prjm_eval_exptreenode_t));

    expr->func = func->func;

    prjm_eval_compiler_node_t* node = calloc(1, sizeof(prjm_eval_compiler_node_t));

    node->type = PRJM_EVAL_NODE_FUNC_EXPRESSION;
    node->instr_is_const_expr = func->is_const_eval;
    node->instr_is_state_changing = func->is_state_changing;
    node->list_is_const_expr = func->is_const_eval;
    node->list_is_state_changing = func->is_state_changing;
    node->tree_node = expr;

    return node;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_create_expression(prjm_eval_compiler_context_t* cctx,
                                                               prjm_eval_function_def_t* func,
                                                               prjm_eval_compiler_arg_list_t* arglist)
{
    prjm_eval_exptreenode_t* expr = calloc(1, sizeof(prjm_eval_exptreenode_t));

    expr->func = func->func;

    /* Need special treatment for memory access functions */
    if (strcmp(func->name, "_mem") == 0 ||
        strcmp(func->name, "megabuf") == 0 ||
        strcmp(func->name, "freembuf") == 0 ||
        strcmp(func->name, "memcpy") == 0 ||
        strcmp(func->name, "memset") == 0)
    {
        expr->memory_buffer = cctx->memory;
    }
    else if (strcmp(func->name, "_gmem") == 0 ||
             strcmp(func->name, "gmegabuf") == 0)
    {
        expr->memory_buffer = cctx->global_memory;
    }

    bool args_are_const_evaluable = true;
    bool args_are_state_changing = false;
    if (arglist && arglist->count > 0)
    {
        expr->args = calloc(arglist->count + 1, sizeof(prjm_eval_exptreenode_t*));

        prjm_eval_compiler_arg_node_t* arg = arglist->begin;
        prjm_eval_exptreenode_t** expr_arg = expr->args;
        while (arg)
        {
            /* Move expression to arguments */
            *expr_arg = arg->node->tree_node;
            arg->node->tree_node = NULL;

            /* If at least one arg is not const-evaluable, the function isn't. */
            args_are_const_evaluable = args_are_const_evaluable && arg->node->list_is_const_expr;
            /* If at least one arg is state-changing, the function is also. */
            args_are_state_changing = args_are_state_changing || arg->node->list_is_state_changing;

            expr_arg++;
            arg = arg->next;
        }
    }

    prjm_eval_compiler_destroy_arglist(arglist);

    prjm_eval_compiler_node_t* node = calloc(1, sizeof(prjm_eval_compiler_node_t));

    node->type = PRJM_EVAL_NODE_FUNC_EXPRESSION;
    node->instr_is_const_expr = args_are_const_evaluable && func->is_const_eval;
    node->instr_is_state_changing = args_are_state_changing || func->is_state_changing;
    node->list_is_const_expr = node->instr_is_const_expr;
    node->list_is_state_changing = node->instr_is_state_changing;
    node->tree_node = expr;

    // Evaluate expression if constant-evaluable
    if (node->instr_is_const_expr &&
        !node->instr_is_state_changing)
    {
        prjm_eval_exptreenode_t* const_expr = calloc(1, sizeof(prjm_eval_exptreenode_t));
        prjm_eval_function_def_t* const_func = prjm_eval_compiler_get_function(cctx, "/*const*/");
        const_expr->func = const_func->func;

        PRJM_EVAL_F* value_ptr = &const_expr->value;
        expr->func(expr, &value_ptr);
        const_expr->value = *value_ptr;

        node->tree_node = const_expr;
        node->instr_is_const_expr = const_func->is_const_eval;
        node->instr_is_state_changing = const_func->is_state_changing;
        node->list_is_const_expr = const_func->is_const_eval;
        node->list_is_state_changing = const_func->is_state_changing;
        prjm_eval_destroy_exptreenode(expr);
    }

    return node;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_create_constant(prjm_eval_compiler_context_t* cctx, PRJM_EVAL_F value)
{
    prjm_eval_function_def_t* const_func = prjm_eval_compiler_get_function(cctx, "/*const*/");

    prjm_eval_exptreenode_t* const_expr = calloc(1, sizeof(prjm_eval_exptreenode_t));
    const_expr->func = const_func->func;
    const_expr->value = value;

    prjm_eval_compiler_node_t* node = calloc(1, sizeof(prjm_eval_compiler_node_t));
    node->type = PRJM_EVAL_NODE_FUNC_EXPRESSION;
    node->tree_node = const_expr;
    node->instr_is_const_expr = const_func->is_const_eval;
    node->instr_is_state_changing = const_func->is_state_changing;
    node->list_is_const_expr = const_func->is_const_eval;
    node->list_is_state_changing = const_func->is_state_changing;

    return node;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_create_variable(prjm_eval_compiler_context_t* cctx, const char* name)
{
    /* Find existing variable or create a new one */
    PRJM_EVAL_F* var = prjm_eval_register_variable(cctx, name);

    prjm_eval_function_def_t* var_func = prjm_eval_compiler_get_function(cctx, "/*var*/");
    prjm_eval_compiler_node_t* node = prjm_eval_compiler_create_expression_empty(var_func);

    node->tree_node->var = var;
    node->instr_is_const_expr = var_func->is_const_eval;
    node->instr_is_state_changing = var_func->is_state_changing;
    node->list_is_const_expr = var_func->is_const_eval;
    node->list_is_state_changing = var_func->is_state_changing;

    return node;
}

prjm_eval_compiler_node_t* prjm_eval_compiler_add_instruction(prjm_eval_compiler_context_t* cctx,
                                                             prjm_eval_compiler_node_t* list,
                                                             prjm_eval_compiler_node_t* instruction)
{
    if (!list)
    {
        return NULL;
    }

    prjm_eval_compiler_node_t* node = list;

    /* Convert single expression to instruction list if needed. */
    if (list->type != PRJM_EVAL_NODE_FUNC_INSTRUCTIONLIST)
    {
        /* If previous instruction is not state-changing, we can remove it as it won't do
         * anything useful. Only the last expression's value may be of interest. */
        if (!list->instr_is_state_changing)
        {
            prjm_eval_compiler_destroy_node(list);
            return instruction;
        }

        prjm_eval_function_def_t* list_func = prjm_eval_compiler_get_function(cctx, "/*list*/");

        prjm_eval_compiler_node_t* new_node = prjm_eval_compiler_create_expression_empty(list_func);
        new_node->tree_node->list = malloc(sizeof(prjm_eval_exptreenode_list_item_t));
        new_node->tree_node->list->expr = list->tree_node;
        new_node->tree_node->list->next = NULL;
        new_node->type = PRJM_EVAL_NODE_FUNC_INSTRUCTIONLIST;
        new_node->instr_is_const_expr = list->instr_is_const_expr;
        new_node->instr_is_state_changing = list->instr_is_state_changing;
        new_node->list_is_const_expr = list->list_is_const_expr;
        new_node->list_is_state_changing = list->list_is_state_changing;

        free(list);

        node = new_node;
    }

    assert(node);
    assert(node->tree_node);
    assert(node->tree_node->list);

    prjm_eval_exptreenode_list_item_t* item = node->tree_node->list;
    while (item->next)
    {
        /* If last expression in the existing list is not state-changing, we can remove it as it won't do
         * anything useful. Only the last expression's value may be of interest. */
        if (!node->instr_is_state_changing && !item->next->next)
        {
            prjm_eval_destroy_exptreenode(item->next->expr);
            free(item->next);
            break;
        }

        item = item->next;
    }

    item->next = malloc(sizeof(prjm_eval_exptreenode_list_item_t));
    item->next->expr = instruction->tree_node;
    item->next->next = NULL;

    /* Update const/state flags of node and list with last expression */
    node->instr_is_const_expr = instruction->list_is_const_expr;
    node->instr_is_state_changing = instruction->list_is_state_changing;
    node->list_is_const_expr = node->list_is_const_expr && instruction->list_is_const_expr;
    node->list_is_state_changing = node->list_is_state_changing || instruction->list_is_state_changing;

    free(instruction);

    return node;
}