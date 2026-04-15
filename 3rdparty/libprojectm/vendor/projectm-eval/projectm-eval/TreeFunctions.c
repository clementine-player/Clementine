/**
 * @file TreeFunctions.c
 * @brief Implements all intrinsic projectM-EvalLib functions and helpers.
 */
#include "TreeFunctions.h"

#include "MemoryBuffer.h"

#include <math.h>
#include <assert.h>
#include <stdint.h>

#if PRJM_F_SIZE == 4
typedef int32_t PRJM_EVAL_I;
#else
typedef int64_t PRJM_EVAL_I;
#endif
/**
 * @brief projectM-EvalLib intrinsic Function table.
 * Contains all predefined functions and information about their invocation. Most functions beginning
 * with _ are not really required because they're internals of ns-eel2, but as they can be used in
 * Milkdrop, we also allow expressions to call them directly.
 */
static prjm_eval_function_def_t intrinsic_function_table[] = {
    /* Special intrinsic functions. Cannot be used via expression syntax. */
    { "/*const*/", prjm_eval_func_const,            0, true,  false },
    { "/*var*/",   prjm_eval_func_var,              0, false, false },
    { "/*list*/",  prjm_eval_func_execute_list,     1, true,  false },
    { "/*or*/",    prjm_eval_func_bitwise_or,       2, true,  false },
    { "/*and*/",   prjm_eval_func_bitwise_and,      2, true,  false },

    { "if",        prjm_eval_func_if,               3, true,  false },
    { "_if",       prjm_eval_func_if,               3, true,  false },
    { "_and",      prjm_eval_func_boolean_and_op,   2, true,  false },
    { "_or",       prjm_eval_func_boolean_or_op,    2, true,  false },
    { "loop",      prjm_eval_func_execute_loop,     2, true,  false },
    { "while",     prjm_eval_func_execute_while,    1, true,  false },

    { "_not",      prjm_eval_func_bnot,             1, true,  false },
    { "bnot",      prjm_eval_func_bnot,             1, true,  false },
    { "_equal",    prjm_eval_func_equal,            2, true,  false },
    { "equal",     prjm_eval_func_equal,            2, true,  false },
    { "_noteq",    prjm_eval_func_notequal,         2, true,  false },
    { "_below",    prjm_eval_func_below,            2, true,  false },
    { "below",     prjm_eval_func_below,            2, true,  false },
    { "_above",    prjm_eval_func_above,            2, true,  false },
    { "above",     prjm_eval_func_above,            2, true,  false },
    { "_beleq",    prjm_eval_func_beloweq,          2, true,  false },
    { "_aboeq",    prjm_eval_func_aboveeq,          2, true,  false },

    { "_set",      prjm_eval_func_set,              2, false, true  },
    { "assign",    prjm_eval_func_set,              2, false, true  },
    { "_add",      prjm_eval_func_add,              2, true,  false },
    { "_sub",      prjm_eval_func_sub,              2, true,  false },
    { "_mul",      prjm_eval_func_mul,              2, true,  false },
    { "_div",      prjm_eval_func_div,              2, true,  false },
    { "_mod",      prjm_eval_func_mod,              2, true,  false },
    { "_mulop",    prjm_eval_func_mul_op,           2, false, true  },
    { "_divop",    prjm_eval_func_div_op,           2, false, true  },
    { "_orop",     prjm_eval_func_bitwise_or_op,    2, false, true  },
    { "_andop",    prjm_eval_func_bitwise_and_op,   2, false, true  },
    { "_addop",    prjm_eval_func_add_op,           2, false, true  },
    { "_subop",    prjm_eval_func_sub_op,           2, false, true  },
    { "_modop",    prjm_eval_func_mod_op,           2, false, true  },

    { "sin",       prjm_eval_func_sin,              1, true,  false },
    { "cos",       prjm_eval_func_cos,              1, true,  false },
    { "tan",       prjm_eval_func_tan,              1, true,  false },
    { "asin",      prjm_eval_func_asin,             1, true,  false },
    { "acos",      prjm_eval_func_acos,             1, true,  false },
    { "atan",      prjm_eval_func_atan,             1, true,  false },
    { "atan2",     prjm_eval_func_atan2,            2, true,  false },
    { "sqr",       prjm_eval_func_sqr,              1, true,  false },
    { "sqrt",      prjm_eval_func_sqrt,             1, true,  false },
    { "pow",       prjm_eval_func_pow,              2, true,  false },
    { "_powop",    prjm_eval_func_pow_op,           2, false, true  },
    { "exp",       prjm_eval_func_exp,              1, true,  false },
    { "_neg",      prjm_eval_func_neg,              1, true,  false },

    { "log",       prjm_eval_func_log,              1, true,  false },
    { "log10",     prjm_eval_func_log10,            1, true,  false },
    { "abs",       prjm_eval_func_abs,              1, true,  false },
    { "min",       prjm_eval_func_min,              2, true,  false },
    { "max",       prjm_eval_func_max,              2, true,  false },
    { "sign",      prjm_eval_func_sign,             1, true,  false },
    { "rand",      prjm_eval_func_rand,             1, false, false },
    { "floor",     prjm_eval_func_floor,            1, true,  false },
    { "int",       prjm_eval_func_floor,            1, true,  false },
    { "ceil",      prjm_eval_func_ceil,             1, true,  false },
    { "invsqrt",   prjm_eval_func_invsqrt,          1, true,  false },
    { "sigmoid",   prjm_eval_func_sigmoid,          2, true,  false },

    { "band",      prjm_eval_func_boolean_and_func, 2, true,  false },
    { "bor",       prjm_eval_func_boolean_or_func,  2, true,  false },

    { "exec2",     prjm_eval_func_exec2,            2, true,  false },
    { "exec3",     prjm_eval_func_exec3,            3, true,  false },
    { "_mem",      prjm_eval_func_mem,              1, false, true  },
    { "megabuf",   prjm_eval_func_mem,              1, false, true  },
    { "_gmem",     prjm_eval_func_mem,              1, false, true  },
    { "gmegabuf",  prjm_eval_func_mem,              1, false, true  },
    { "freembuf",  prjm_eval_func_freembuf,         1, false, true  },
    { "memcpy",    prjm_eval_func_memcpy,           3, false, true  },
    { "memset",    prjm_eval_func_memset,           3, false, true  }
};


/* Helper functions and defines */
#define assert_valid_ctx() \
        assert(ctx); \
        assert(ret_val); \
        assert(*ret_val); \
        assert(ctx->func)

/* Allowed error for float/double comparisons to exact values */
#define COMPARE_CLOSEFACTOR 0.00001
static const PRJM_EVAL_F close_factor = COMPARE_CLOSEFACTOR;

/* These factors are not exactly as close to zero as their ns-eel2 equivalents, but that shouldn't
 * matter too much. In ns-eel2, the value is represented as binary 0x00000000FFFFFFFF for doubles.
 */
#if PRJM_F_SIZE == 4
static const PRJM_EVAL_F close_factor_low = 1e-41;
#else
static const PRJM_EVAL_F close_factor_low = 1e-300;
#endif

/* Maximum number of loop iterations */
#define MAX_LOOP_COUNT 1048576

/**
 * Invokes the expression function of the indexed argument
 */
#define invoke_arg(argnum, retval) \
    assert(ctx->args[argnum]);    \
    ctx->args[argnum]->func(ctx->args[argnum], retval)

/**
 * Assigns the value as a constant PRJM_F return value.
 * @param value The expression or value to assign. Must evaluate to PRJM_F.
 */
#define assign_ret_val(value) \
    (**ret_val) = value

/**
 * Assigns the value as a constant PRJM_F return value.
 * @param ref The reference pointer to assign. Must evaluate to PRJM_F*.
 */
#define assign_ret_ref(ref) \
    (*ret_val) = ref

/* Used in genrand_int32 */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

void prjm_eval_intrinsic_functions(prjm_eval_intrinsic_function_list_ptr list, uint32_t* count)
{
    *count = sizeof(intrinsic_function_table) / sizeof(prjm_eval_function_def_t);
    *list = intrinsic_function_table;
}

/* This is Milkdrop's original rand() implementation. */
static uint32_t prjm_eval_genrand_int32(void)
{
    uint32_t y;
    static uint32_t mag01[2] = { 0x0UL, MATRIX_A };
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    static uint32_t mt[N]; /* the array for the state vector  */
    static int32_t mti; /* mti==N+1 means mt[N] is not initialized */


    if (!mti)
    {
        uint32_t s = 0x4141f00d; // Initial Mersenne Twister seed
        mt[0] = s & 0xffffffffUL;
        for (mti = 1; mti < N; mti++)
        {
            mt[mti] =
                (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);

            /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
            /* In the previous versions, MSBs of the seed affect   */
            /* only MSBs of the array mt[].                        */
            /* 2002/01/09 modified by Makoto Matsumoto             */
            mt[mti] &= 0xffffffffUL;
            /* for >32 bit machines */
        }
    }

    if (mti >= N)
    { /* generate N words at one time */
        int32_t kk;

        for (kk = 0; kk < N - M; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* Variable and constant access */
prjm_eval_function_decl(const)
{
    assert_valid_ctx();

    assign_ret_val(ctx->value);
}

prjm_eval_function_decl(var)
{
    assert_valid_ctx();
    assert(ctx->var);

    assign_ret_ref(ctx->var);
}


/* Execution control structures */
prjm_eval_function_decl(execute_list)
{
    assert_valid_ctx();
    assert(ctx->list);

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;
    prjm_eval_exptreenode_list_item_t* item = ctx->list;
    while (item)
    {
        assert(item->expr);
        assert(item->expr->func);

        ctx->value = .0;
        value_ptr = &ctx->value;
        item->expr->func(item->expr, &value_ptr);
        item = item->next;
    }

    assign_ret_ref(value_ptr);
}

prjm_eval_function_decl(execute_loop)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;
    invoke_arg(0, &value_ptr);

    PRJM_EVAL_I loop_count_int = (PRJM_EVAL_I) (*value_ptr);
    /* Limit execution count */
    if (loop_count_int > MAX_LOOP_COUNT)
    {
        loop_count_int = MAX_LOOP_COUNT;
    }

    for (PRJM_EVAL_I i = 0; i < loop_count_int; i++)
    {
        ctx->value = .0;
        value_ptr = &ctx->value;
        invoke_arg(1, &value_ptr);
    }

    assign_ret_ref(value_ptr);
}

prjm_eval_function_decl(execute_while)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;
    PRJM_EVAL_I loop_count_int = MAX_LOOP_COUNT;
    do
    {
        invoke_arg(0, &value_ptr);
    } while (fabs(*value_ptr) > close_factor_low && --loop_count_int);

    assign_ret_ref(value_ptr);
}

prjm_eval_function_decl(if)
{
    assert_valid_ctx();

    PRJM_EVAL_F* if_arg = &ctx->value;

    invoke_arg(0, &if_arg);

    if ((*if_arg) != 0)
    {
        invoke_arg(1, ret_val);
        return;
    }
    invoke_arg(2, ret_val);
}

prjm_eval_function_decl(exec2)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);
    invoke_arg(1, ret_val);
}

prjm_eval_function_decl(exec3)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);
    invoke_arg(1, &value_ptr);
    invoke_arg(2, ret_val);
}

prjm_eval_function_decl(set)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, ret_val);
    invoke_arg(1, &value_ptr);

    assign_ret_val(*value_ptr);
}


/* Memory access functions */
prjm_eval_function_decl(mem)
{
    assert_valid_ctx();
    assert(ctx->memory_buffer);

    ctx->value = .0;
    PRJM_EVAL_F* index_ptr = &ctx->value;
    invoke_arg(0, &index_ptr);

    // Add 0.0001 to avoid using the wrong index due to tiny float rounding errors.
    PRJM_EVAL_F* mem_addr = prjm_eval_memory_allocate(ctx->memory_buffer, (int32_t) (*index_ptr + 0.0001));
    if (mem_addr)
    {
        assign_ret_ref(mem_addr);
        return;
    }

    assign_ret_val(.0);
}

prjm_eval_function_decl(freembuf)
{
    assert_valid_ctx();
    assert(ctx->memory_buffer);

    invoke_arg(0, ret_val);

    // Add 0.0001 to avoid using the wrong index due to tiny float rounding errors.
    prjm_eval_memory_free_block(ctx->memory_buffer, (int32_t) (**ret_val + 0.0001));
}

prjm_eval_function_decl(memcpy)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F src_index = .0;
    PRJM_EVAL_F count = .0;
    PRJM_EVAL_F* dest_index_ptr = &ctx->value;
    PRJM_EVAL_F* src_index_ptr = &src_index;
    PRJM_EVAL_F* count_ptr = &count;

    invoke_arg(0, &dest_index_ptr);
    invoke_arg(1, &src_index_ptr);
    invoke_arg(2, &count_ptr);

    assign_ret_ref(prjm_eval_memory_copy(ctx->memory_buffer, dest_index_ptr, src_index_ptr, count_ptr));
}

prjm_eval_function_decl(memset)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F value = .0;
    PRJM_EVAL_F count = .0;
    PRJM_EVAL_F* dest_index_ptr = &ctx->value;
    PRJM_EVAL_F* value_ptr = &value;
    PRJM_EVAL_F* count_ptr = &count;

    invoke_arg(0, &dest_index_ptr);
    invoke_arg(1, &value_ptr);
    invoke_arg(2, &count_ptr);

    assign_ret_ref(prjm_eval_memory_set(ctx->memory_buffer, dest_index_ptr, value_ptr, count_ptr));
}



/* Operators */

prjm_eval_function_decl(bnot)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    assign_ret_val(fabs(*value_ptr) < close_factor_low ? 1.0 : 0.0);
}

prjm_eval_function_decl(equal)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(fabs(*val1_ptr - *val2_ptr) < close_factor_low ? 1.0 : 0.0);
}

prjm_eval_function_decl(notequal)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;
    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(fabs(*val1_ptr - *val2_ptr) > close_factor_low ? 1.0 : 0.0);
}

prjm_eval_function_decl(below)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((*val1_ptr < *val2_ptr) ? 1.0 : 0.0);
}

prjm_eval_function_decl(above)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((*val1_ptr > *val2_ptr) ? 1.0 : 0.0);
}

prjm_eval_function_decl(beloweq)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((*val1_ptr <= *val2_ptr) ? 1.0 : 0.0);
}

prjm_eval_function_decl(aboveeq)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((*val1_ptr >= *val2_ptr) ? 1.0 : 0.0);
}

prjm_eval_function_decl(add)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(*val1_ptr + *val2_ptr);
}

prjm_eval_function_decl(sub)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(*val1_ptr - *val2_ptr);
}

prjm_eval_function_decl(mul)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(*val1_ptr * *val2_ptr);
}

prjm_eval_function_decl(div)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    if(fabs(*val2_ptr) < close_factor_low)
    {
        assign_ret_val(0.0);
        return;
    }

    assign_ret_val(*val1_ptr / *val2_ptr);
}

prjm_eval_function_decl(mod)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    PRJM_EVAL_I divisor = (PRJM_EVAL_I) *val2_ptr;
    if (divisor == 0)
    {
        assign_ret_val(0.0);
        return;
    }
    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I) *val1_ptr % divisor));
}

prjm_eval_function_decl(boolean_and_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    /*
     * The && operator differs from the "band" function as it only evaluates the second
     * argument if the first one is non-zero.
     */
    invoke_arg(0, &val1_ptr);

    if (fabs(*val1_ptr) > close_factor_low)
    {
        invoke_arg(1, &val2_ptr);

        assign_ret_val(fabs(*val2_ptr) > close_factor_low ? 1.0 : 0.0);
    }
    else
    {
        assign_ret_val(0.0);
    }
}

prjm_eval_function_decl(boolean_or_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    /*
     * The || operator differs from the "bor" function as it only evaluates the second
     * argument if the first one is zero.
     */
    invoke_arg(0, &val1_ptr);

    if (fabs(*val1_ptr) < close_factor_low)
    {
        invoke_arg(1, &val2_ptr);

        assign_ret_val(fabs(*val2_ptr) > close_factor_low ? 1.0 : 0.0);
    }
    else
    {
        assign_ret_val(1.0);
    }
}

prjm_eval_function_decl(boolean_and_func)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    /* This function also uses the larger close factor! */
    assign_ret_val(fabs(*val1_ptr) > close_factor && fabs(*val2_ptr) > close_factor ? 1.0 : 0.0);
}

prjm_eval_function_decl(boolean_or_func)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    /* This function also uses the larger close factor! */
    assign_ret_val(fabs(*val1_ptr) > close_factor || fabs(*val2_ptr) > close_factor ? 1.0 : 0.0);
}

prjm_eval_function_decl(neg)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;

    invoke_arg(0, &val1_ptr);

    assign_ret_val(-(*val1_ptr));
}

prjm_eval_function_decl(add_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(**ret_val + *val2_ptr);
}

prjm_eval_function_decl(sub_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(**ret_val - *val2_ptr);
}

prjm_eval_function_decl(mul_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    assign_ret_val(**ret_val * *val2_ptr);
}

prjm_eval_function_decl(div_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    if(fabs(*val2_ptr) < close_factor_low)
    {
        assign_ret_val(0.0);
        return;
    }

    assign_ret_val(**ret_val / *val2_ptr);
}

prjm_eval_function_decl(bitwise_or_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I)(**ret_val) | (PRJM_EVAL_I)(*val2_ptr)));
}

prjm_eval_function_decl(bitwise_or)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I)(*val1_ptr) | (PRJM_EVAL_I)(*val2_ptr)));
}

prjm_eval_function_decl(bitwise_and_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I)(**ret_val) & (PRJM_EVAL_I)(*val2_ptr)));
}

prjm_eval_function_decl(bitwise_and)
{
    assert_valid_ctx();

    PRJM_EVAL_F val1 = .0;
    PRJM_EVAL_F* val1_ptr = &val1;
    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, &val1_ptr);
    invoke_arg(1, &val2_ptr);

    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I)(*val1_ptr) & (PRJM_EVAL_I)(*val2_ptr)));
}

prjm_eval_function_decl(mod_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    PRJM_EVAL_I divisor = (PRJM_EVAL_I) *val2_ptr;
    if (divisor == 0)
    {
        assign_ret_val(0.0);
        return;
    }
    assign_ret_val((PRJM_EVAL_F) ((PRJM_EVAL_I)(**ret_val) % divisor));
}

prjm_eval_function_decl(pow_op)
{
    assert_valid_ctx();

    PRJM_EVAL_F val2 = .0;
    PRJM_EVAL_F* val2_ptr = &val2;

    invoke_arg(0, ret_val);
    invoke_arg(1, &val2_ptr);

    if(fabs(**ret_val) < close_factor_low && *val2_ptr < 0)
    {
        assign_ret_val(.0);
        return;
    }

    PRJM_EVAL_F result = pow(**ret_val, *val2_ptr);

    assign_ret_val(isnan(result) ? .0 : result);
}


/* Math functions */
prjm_eval_function_decl(sin)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(sin(*math_arg_ptr));
}

prjm_eval_function_decl(cos)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(cos(*math_arg_ptr));
}

prjm_eval_function_decl(tan)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(tan(*math_arg_ptr));
}

prjm_eval_function_decl(asin)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    if (*math_arg_ptr < -1.0 || *math_arg_ptr > 1.0)
    {
        assign_ret_val(.0);
        return;
    }

    assign_ret_val(asin(*math_arg_ptr));
}

prjm_eval_function_decl(acos)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    if (*math_arg_ptr < -1.0 || *math_arg_ptr > 1.0)
    {
        assign_ret_val(.0);
        return;
    }

    assign_ret_val(acos(*math_arg_ptr));
}

prjm_eval_function_decl(atan)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(atan(*math_arg_ptr));
}

prjm_eval_function_decl(atan2)
{
    assert_valid_ctx();

    PRJM_EVAL_F math_arg1 = .0;
    PRJM_EVAL_F math_arg2 = .0;
    PRJM_EVAL_F* math_arg1_ptr = &math_arg1;
    PRJM_EVAL_F* math_arg2_ptr = &math_arg2;

    invoke_arg(0, &math_arg1_ptr);
    invoke_arg(1, &math_arg2_ptr);

    assign_ret_val(atan2(*math_arg1_ptr, *math_arg2_ptr));
}

prjm_eval_function_decl(sqrt)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(sqrt(fabs(*math_arg_ptr)));
}

prjm_eval_function_decl(pow)
{
    assert_valid_ctx();

    PRJM_EVAL_F math_arg1 = .0;
    PRJM_EVAL_F math_arg2 = .0;
    PRJM_EVAL_F* math_arg1_ptr = &math_arg1;
    PRJM_EVAL_F* math_arg2_ptr = &math_arg2;

    invoke_arg(0, &math_arg1_ptr);
    invoke_arg(1, &math_arg2_ptr);

    if (fabs(*math_arg1_ptr) < close_factor_low && *math_arg2_ptr < 0)
    {
        assign_ret_val(.0);
        return;
    }

    PRJM_EVAL_F result = pow(*math_arg1_ptr, *math_arg2_ptr);

    assign_ret_val(isnan(result) ? .0 : result);
}

prjm_eval_function_decl(exp)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(exp(*math_arg_ptr));
}

prjm_eval_function_decl(log)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    if (*math_arg_ptr <= 0.0)
    {
        assign_ret_val(.0);
        return;
    }

    assign_ret_val(log(*math_arg_ptr));
}

prjm_eval_function_decl(log10)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    if (*math_arg_ptr <= 0.0)
    {
        assign_ret_val(.0);
        return;
    }

    assign_ret_val(log10(*math_arg_ptr));
}

prjm_eval_function_decl(floor)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(floor(*math_arg_ptr));
}

prjm_eval_function_decl(ceil)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* math_arg_ptr = &ctx->value;

    invoke_arg(0, &math_arg_ptr);

    assign_ret_val(ceil(*math_arg_ptr));
}

prjm_eval_function_decl(sigmoid)
{
    assert_valid_ctx();

    PRJM_EVAL_F math_arg1 = .0;
    PRJM_EVAL_F math_arg2 = .0;
    PRJM_EVAL_F* math_arg1_ptr = &math_arg1;
    PRJM_EVAL_F* math_arg2_ptr = &math_arg2;

    invoke_arg(0, &math_arg1_ptr);
    invoke_arg(1, &math_arg2_ptr);

    double t = (1 + exp((double) -(*math_arg1_ptr) * (*math_arg2_ptr)));
    assign_ret_val((PRJM_EVAL_F) (fabs(t) > close_factor ? 1.0 / t : .0));
}

prjm_eval_function_decl(sqr)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    assign_ret_val((*value_ptr) * (*value_ptr));
}

prjm_eval_function_decl(abs)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    assign_ret_val(fabs(*value_ptr));
}

prjm_eval_function_decl(min)
{
    assert_valid_ctx();

    PRJM_EVAL_F math_arg1 = .0;
    PRJM_EVAL_F math_arg2 = .0;
    PRJM_EVAL_F* math_arg1_ptr = &math_arg1;
    PRJM_EVAL_F* math_arg2_ptr = &math_arg2;

    invoke_arg(0, &math_arg1_ptr);
    invoke_arg(1, &math_arg2_ptr);

    assign_ret_val((*math_arg1_ptr) < (*math_arg2_ptr) ? (*math_arg1_ptr) : (*math_arg2_ptr));
}

prjm_eval_function_decl(max)
{
    assert_valid_ctx();

    PRJM_EVAL_F math_arg1 = .0;
    PRJM_EVAL_F math_arg2 = .0;
    PRJM_EVAL_F* math_arg1_ptr = &math_arg1;
    PRJM_EVAL_F* math_arg2_ptr = &math_arg2;

    invoke_arg(0, &math_arg1_ptr);
    invoke_arg(1, &math_arg2_ptr);

    assign_ret_val((*math_arg1_ptr) > (*math_arg2_ptr) ? (*math_arg1_ptr) : (*math_arg2_ptr));
}

prjm_eval_function_decl(sign)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    if (*value_ptr == 0)
    {
        (**ret_val) = .0;
        return;
    }
    assign_ret_val((*value_ptr) < .0 ? -1. : 1.);
}

prjm_eval_function_decl(rand)
{
    assert_valid_ctx();

    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    PRJM_EVAL_F rand_max = floor(*value_ptr);
    if (rand_max < 1.)
    {
        rand_max = 1.;
    }

    assign_ret_val((PRJM_EVAL_F) (prjm_eval_genrand_int32() * (1.0 / (double) 0xFFFFFFFF) * rand_max));
}

prjm_eval_function_decl(invsqrt)
{
    assert_valid_ctx();

    /*
     * Using fast inverse square root implementation here, same as Milkdrop, except supporting doubles.
     * See https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
#if PRJM_F_SIZE == 4
#define INVSQRT_MAGIC_NUMBER 0x5f3759df
#define INVSQRT_INT uint32_t
#else
#define INVSQRT_MAGIC_NUMBER 0x5fe6eb50c7b537a9
#define INVSQRT_INT uint64_t
#endif

    union
    {
        PRJM_EVAL_F PRJM_F_val;
        INVSQRT_INT int_val;
    } type_conv;

    static const PRJM_EVAL_F three_halfs = 1.5;
    static const PRJM_EVAL_F one_half = .5;


    ctx->value = .0;
    PRJM_EVAL_F* value_ptr = &ctx->value;

    invoke_arg(0, &value_ptr);

    PRJM_EVAL_F num2 = (*value_ptr) * one_half;
    type_conv.PRJM_F_val = (*value_ptr);
    type_conv.int_val = INVSQRT_MAGIC_NUMBER - (type_conv.int_val >> 1);
    type_conv.PRJM_F_val = type_conv.PRJM_F_val * (three_halfs - (num2 * type_conv.PRJM_F_val * type_conv.PRJM_F_val));

    assign_ret_val(isnan(type_conv.PRJM_F_val) ? 0 : (type_conv.PRJM_F_val));
}
