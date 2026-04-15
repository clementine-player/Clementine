Compiler Internals
==================

This document describes how the compiler works and how its functionality can be extended. It is aimed at developers
integrating projectM-EvalLib into their own applications and looking for adding functions and new features on top of the
supported set of Milkdrop-compatible expression syntax.

## Scanner

The scanner (or lexer) is responsible for tokenizing the input data stream and return the type and possible content of
these tokens to the parser. This library uses GNU Flex to generate ode from a rules file, called `Scanner.l`. The
scanner looks for these kinds of symbols, with letters always being case-insensitive:

- Inline comments, starting with `//` until end of the current line
- Block comments, enclosed in `/* ... */`, which can appear anywhere and also span multiple lines.
- HEX constants in the form `$Xnn...`, scanned until any non-hexadecimal character is encountered (`[0-9a-f]`)
- ORD constants in the form `$'c'` where `c` is any single character.
- Math constants `$PI`, `$E` and `$PHI`.
- The `gmem` identifier, as it's used in the non-function form `gmem[index]`, requiring special treatment in the
  grammar.
- Numbers in integer and floating-point notation with an optional exponent. Note that these numbers are always positive,
  negativity is applied later via the `-` unary operator in the parser.
- Identifiers starting with an underscore or alphabetical letter, and also numbers on any following character.
  These can be returned as two different tokens:
    - If the (lower-case) name exists in the function table, the name is returned as a `FUN` token.
    - Anything else is returned as a `VAR` token and identifies a variable.

## Grammar

The grammar is defined in Bison (yacc) syntax and specifies the order in which tokens can appear and how they are
transformed into a tree structure.

In the grammar, different syntactical features are specified:

- Tokens returned by the scanner and their associated C data types in the parser.
- Operator precedence and associativity: Determines which operators are evaluated before others, and in which
  direction (left-to-right or right-to-left).
- The recursively defined grammar, consisting of a list of tokens per rule and an action.

Bison creates a state machine from the grammar, analyze any issues like potential ambiguities and take care of the
proper shift/reduce operations. Bison also provides error handling.

### Compilation Errors

In this grammar, any syntax error encountered will fail the compilation of the whole program. __This is a difference in
comparison to the Milkdrop parser__, which will not fail compilation on certain circumstances, silently ignoring errors
in the code. Reproducing the exact same behaviour would require a completely different approach in the grammar, scanner
and parser.

## Expression Tree

### Node Object

All grammar as described in the previous sections is scanned and compiled into a tree structure by the parser actions.
Each node is a `prjm_eval_exptreenode` struct, which contains:

- A function pointer `func`, which is determined the behaviour of this node.
- A fixed float value `value` to store constant numbers or temporary values.
- A union of either `var`, pointing to a variable storage location, or `memory_buffer` which is a pointer to
  a `projectm_eval_mem_buffer` with (g)megabuf data.
- An array of pointers `args`, pointing to the argument node objects of the function.
- A linked-list pointer `list` which stores an instruction list to be executed by loops or the special `/*list*/`"
  function.

In general, only a few different "objects" will be created as tree nodes:

- Constant: Return simple read-only float-typed numbers. Uses the `value` member to store the constant.
- Variable: Return a read/write variable reference. USes the `var` member to store the variable storage location.
- Memory access function: Operates on the memory buffer specified in `memory_buffer`, either retrieving or setting data.
- Function: Executes objects in `args` and/or the expressions in `list`, then eventually applies an operation on the
  result and sets the return value to either a constant value or a variable reference.

### Node Function

The function in each node object has a `void` return value and takes two parameters:

- The execution context `ctx`, which is a pointer to the `prjm_eval_exptreenode` object the function will be executed on.
  Think of it as the C++ `this` pointers.
- A pointer to a float pointer `ret_val`, which is the return value of the function.

Every node function _must_ return either a value or a value reference. By convention, this is either the result of the
operation the function carried out, or for functions using instruction lists, the result of the _last executed
instruction_.

Depending on the function, the result can either be a simple constant value or a reference to a variable.

When calling a math function like `sin()`, implementing a math operator or returning a constant, the function must
assign the value to the value pointed to by `*ret_val`. Use the `assign_ret_val()` macro to assign the return value:

```c
assign_ret_val(sin(x));
```

Some functions will not carry out an actual math operation, but return the result of an expression, which may be a
variable reference, which may be used as the first argument of the internal `_set` function (or the
AVS-specific `assign` function) to assign it the result of another expression. A reference can be assigned using
the `assign_ret_ref` macro, example taken from the `prjm_eval_func_var` function:

```c
assign_ret_ref(ctx->var);
```

As with all pointers, an assigned pointer must stay valid at least until the end of the program's execution. It is
recommended to only use the following memory references:

- The `var` pointer in variable access functions.
- The address of the `value` member of the current `prjm_eval_exptreenode` object in which context the function runs.

Do not assign pointers to locally defined variables.

Passing `ret_val` as the return value to a function argument and let the function set it to the desired result is also
viable:

```C
prjm_eval_function_decl(myfunc)
{
    assert_valid_ctx();

    invoke_arg(0, ret_val);
}
```

The `if` function evaluates its first argument, and depending on it being a true or false value, returns either the
second (`true`) or the third (`false`) argument. Both arguments may return a variable reference, so the `if` functions
needs to take care of this by simply calling the appropriate function and passing the current `ret_val` to it. This way,
whatever the sub-expression assign to it will be returned by the `if` function.

### Temporary Compiler Objects

#### Node Object

Any tree node object created in a parser action will be wrapped inside a temporary compiler node of
type `prjm_eval_compiler_node`, which stores a few additional flags only required inside the parser's reduce actions:

- The type of the current parser node, either a single expression or an expression list.
- Constness and state-changing flags of the expression (last expression in a list).
- combined constness and state-changing flags of the expression list. Same as the previous one if only a single
  expression is stored in the compiler node.

#### Function Argument List

Inside a function argument list, all expressions are collected in a special `prjm_eval_compiler_arg_list` wrapper holding
one compiler node object for each potential function argument. It also keeps the argument count as a separate variable
for easy access.

When the arguments have been collected and the function is reduced in the parser, the action will then compare the
actual argument count against the count expected by the function. If the numbers don't match, a parse error is thrown.

### Optimizations

The parser does perform two different optimizations during compile time to save execution time:

#### Compile-time Evaluable Functions

The compiler optimizes the generated code by evaluating constant expressions at compile time. These expressions purely
consist of functions which both only have constant-evaluable arguments and have a fixed, deterministic behaviour.
Examples of such functions, given their arguments are also const-evaluable:

- Functions which return compile-time constants.
- Non-assigment Operators.
- Math functions like `sin`.

Non-constant functions are functions which depend on variables or other memory contents which may change during or
between executions or produce a non-deterministic result on each execution. Examples are:

- Functions which have at least one non-const-evaluable argument.
- Variable and memory access functions.
- Random number generators.

During compilation, when a new node is created from a subtree, each argument and the new node are checked for being
const-evaluable. If all are, then the new node is immediately evaluated by calling the function, and a new constant node
is inserted with the calculated value, replacing both the currently processed node and its argument nodes.

#### Expression List Optimization

In function arguments and inside parentheses, expression lists, e.g. multiple expressions separated by semicolons, can
be written. This is a useful feature to execute a list of expressions based on certain conditions (e.g. in `if`
or `loop`). While only the _last_ expression in a list determines the return value of the whole list, other expressions
may perform calculations, set variables etc. and also need to be executed in order to achieve the correct result.

There could also be expressions in the list which do not assign anything, and which are not the last expressions. These
expressions are referred to as "non state-changing", and only waste CPU cycles because the result of any calculation
done there is discarded. The expression compiler will keep track of the state-changing flag hierarchically, similar to
the const flag, and remove any expressions which are nether state-changing nor relevant to the return value.

In the case that a state-changing operator, e.g. an assignment, has a constant value as the left operand, it will be
replaced by the const-evaluation optimization and lose its state-changing flag. Consider the following expression, where
the first two expressions will be optimized out:

```
sin(1 + 2; 3);
tan(5) = x;
5 = rand(100);
x = cos($PI * .54);
5;
```

The third expression cannot be optimized out, because the assignment operator is potentially state-changing if it
assigns to a variable or memory location and the `rand` function is not const-evaluable.

The fourth expression is very similar, but this time, the use of a variable prevents the expression form being
const-evaluable.

The fifth and last expression is a simple constant and determines the return value of the whole expression list.

