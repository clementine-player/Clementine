/**
 * @file TreeFunctions.h
 * @brief Declares all intrinsic projectM-EvalLib functions and types.
 */
#pragma once

#include "CompilerTypes.h"

#include <stdint.h>

/**
 * @brief Returns the list with the built-in function table.
 * The list must not be freed, as it points to the internal static data.
 * @param list A pointer which will receive the list reference.
 * @param count The number of elements inserted into the list.
 */
void prjm_eval_intrinsic_functions(prjm_eval_intrinsic_function_list_ptr list, uint32_t* count);

/**
 * @brief Abbreviates parser function declaration.
 * @param func The name of the function, appended to "prjm_eval_func_".
 */
#define prjm_eval_function_decl(func) \
    void prjm_eval_func_ ## func(struct prjm_eval_exptreenode* ctx, PRJM_EVAL_F** ret_val)

/* Variable and constant access */
prjm_eval_function_decl(const);
prjm_eval_function_decl(var);

/* Execution control structures */
prjm_eval_function_decl(execute_list);
prjm_eval_function_decl(execute_loop);
prjm_eval_function_decl(execute_while);
prjm_eval_function_decl(if);
prjm_eval_function_decl(exec2);
prjm_eval_function_decl(exec3);

/* Assignment function */
prjm_eval_function_decl(set);

/* Memory access function */
prjm_eval_function_decl(mem);
prjm_eval_function_decl(freembuf);
prjm_eval_function_decl(memcpy);
prjm_eval_function_decl(memset);

/* Operators */
prjm_eval_function_decl(bnot);
prjm_eval_function_decl(equal);
prjm_eval_function_decl(notequal);
prjm_eval_function_decl(below);
prjm_eval_function_decl(above);
prjm_eval_function_decl(beloweq);
prjm_eval_function_decl(aboveeq);
prjm_eval_function_decl(add);
prjm_eval_function_decl(sub);
prjm_eval_function_decl(mul);
prjm_eval_function_decl(div);
prjm_eval_function_decl(mod);
prjm_eval_function_decl(bitwise_or);
prjm_eval_function_decl(bitwise_and);
prjm_eval_function_decl(boolean_and_op);
prjm_eval_function_decl(boolean_or_op);
prjm_eval_function_decl(boolean_and_func);
prjm_eval_function_decl(boolean_or_func);
prjm_eval_function_decl(neg);

/* Compound assignment operators */
prjm_eval_function_decl(add_op);
prjm_eval_function_decl(sub_op);
prjm_eval_function_decl(mul_op);
prjm_eval_function_decl(div_op);
prjm_eval_function_decl(mod_op);
prjm_eval_function_decl(bitwise_or_op);
prjm_eval_function_decl(bitwise_and_op);
prjm_eval_function_decl(pow_op);

/* Mathematical functions */
prjm_eval_function_decl(sin);
prjm_eval_function_decl(cos);
prjm_eval_function_decl(tan);
prjm_eval_function_decl(asin);
prjm_eval_function_decl(acos);
prjm_eval_function_decl(atan);
prjm_eval_function_decl(atan2);
prjm_eval_function_decl(sqrt);
prjm_eval_function_decl(pow);
prjm_eval_function_decl(exp);
prjm_eval_function_decl(log);
prjm_eval_function_decl(log10);
prjm_eval_function_decl(floor);
prjm_eval_function_decl(ceil);
prjm_eval_function_decl(sigmoid);
prjm_eval_function_decl(sqr);
prjm_eval_function_decl(abs);
prjm_eval_function_decl(min);
prjm_eval_function_decl(max);
prjm_eval_function_decl(sign);
prjm_eval_function_decl(rand);
prjm_eval_function_decl(invsqrt);
