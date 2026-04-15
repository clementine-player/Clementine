#pragma once

#include "api/projectm-eval.h"

#include <stdbool.h>

struct prjm_eval_exptreenode;

/**
 * @brief Node function for a single expression.
 */
typedef void (prjm_eval_expr_func_t)(struct prjm_eval_exptreenode* ctx, PRJM_EVAL_F** ret_val);

/**
 * @brief Structure containing information about an available function implementation.
 * This struct is used to fill the intrinsic function table and is used to add additional,
 * externally-defined functions to the parser.
 */
typedef struct prjm_eval_function_def
{
    char* name; /*!< The lower-case name of the function in the expression syntax */
    prjm_eval_expr_func_t* func; /*!< A pointer to the function implementation */
    int arg_count; /*!< Number of accepted parameters, 1 to 3. */
    bool is_const_eval; /*!< If true, the function can be evaluated to a constant value at compile time. */
    bool is_state_changing; /*!< If true, the function will change the execution state (set memory) */
} prjm_eval_function_def_t;

typedef struct prjm_eval_function_list_item
{
    prjm_eval_function_def_t* function;
    struct prjm_eval_function_list_item* next;
} prjm_eval_function_list_item_t;

typedef struct
{
    prjm_eval_function_list_item_t* first;
} prjm_eval_function_list_t;

typedef const prjm_eval_function_def_t* prjm_eval_intrinsic_function_list;
typedef prjm_eval_intrinsic_function_list* prjm_eval_intrinsic_function_list_ptr;


typedef struct prjm_eval_variable_def
{
    char* name; /*!< The lower-case name of the variable in the expression syntax. */
    PRJM_EVAL_F value; /*!< The internal value of the variable. */
} prjm_eval_variable_def_t;

typedef struct prjm_eval_variable_entry
{
    prjm_eval_variable_def_t* variable;
    struct prjm_eval_variable_entry* next;
} prjm_eval_variable_entry_t;

typedef struct
{
    prjm_eval_variable_entry_t* first;
} prjm_eval_variable_list_t;

struct prjm_eval_exptreenode;

typedef struct prjm_eval_exptreenode_list_item
{
    struct prjm_eval_exptreenode* expr;
    struct prjm_eval_exptreenode_list_item* next;
} prjm_eval_exptreenode_list_item_t;

/**
 * @brief A single function, variable or constant in the expression tree.
 * The assigned function will determine how to access the other members.
 */
typedef struct prjm_eval_exptreenode
{
    prjm_eval_expr_func_t* func;
    PRJM_EVAL_F value; /*!< A constant, numerical value. Also used as temp value. */
    union
    {
        PRJM_EVAL_F* var; /*!< Variable reference. */
        projectm_eval_mem_buffer memory_buffer; /*!< megabuf/gmegabuf memory block. */
    };
    struct prjm_eval_exptreenode** args; /*!< Function arguments. Last element must be a NULL pointer*/
    prjm_eval_exptreenode_list_item_t* list;  /*!< Next argument in the instruction list. */
} prjm_eval_exptreenode_t;


typedef enum prjm_eval_compiler_node_type
{
    PRJM_EVAL_NODE_FUNC_EXPRESSION,
    PRJM_EVAL_NODE_FUNC_INSTRUCTIONLIST
} prjm_eval_compiler_node_type_t;


typedef struct prjm_eval_compiler_node
{
    prjm_eval_compiler_node_type_t type; /*!< Node type. Mostly expression, but can contain other types as well. */
    prjm_eval_exptreenode_t* tree_node; /*!< Generated tree node for this expression */
    bool instr_is_const_expr; /*!< If true, this node and all sub nodes only consist of constant expressions, e.g. no variables used */
    bool instr_is_state_changing; /*!< If true, the function will change the execution state (set memory) */
    bool list_is_const_expr; /*!< If true, the instruction list only consists of constant expressions, e.g. no variables used */
    bool list_is_state_changing; /*!< If true, at least one node in the instruction list will change the execution state (set memory) */
} prjm_eval_compiler_node_t;

typedef struct prjm_eval_compiler_arg_item
{
    prjm_eval_compiler_node_t* node; /*!< Expression for this argument. */
    struct prjm_eval_compiler_arg_item* next; /*!< Next argument in list. */
} prjm_eval_compiler_arg_node_t;

typedef struct prjm_eval_compiler_arg_list
{
    int count; /*!< Argument count in this list */
    prjm_eval_compiler_arg_node_t* begin; /*!< First argument in the list. */
    prjm_eval_compiler_arg_node_t* end; /*!< Last argument in the list. */
} prjm_eval_compiler_arg_list_t;

typedef struct
{
    char* error;
    int line;
    int column_start;
    int column_end;
} prjm_eval_compiler_error_t;

typedef struct projectm_eval_context
{
    prjm_eval_function_list_t functions; /*!< Functions available to this context. Initialized with the intrinsics table. */
    prjm_eval_variable_list_t variables; /*!< List of registered variables in this context. */
    PRJM_EVAL_F (*global_variables)[100]; /*!< Pointer to array with 100 global variables, reg00 to reg99. */
    projectm_eval_mem_buffer memory; /*!< The context-local memory buffer, referred to as megabuf. */
    projectm_eval_mem_buffer global_memory; /*!< The global memory buffer, referred to as gmegabuf. */
    prjm_eval_compiler_error_t error; /*!< Holds information about the last compile error. */
    prjm_eval_exptreenode_t* compile_result; /*!< The result of the last compilation. Used temporarily during compilation. */
} prjm_eval_compiler_context_t;

typedef struct
{
    prjm_eval_exptreenode_t* program;
    prjm_eval_compiler_context_t* cctx;
} prjm_eval_program_t;
