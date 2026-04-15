Milkdrop Preset Expression Syntax
=================================

This document contains a full writeup of the expression syntax in Milkdrop presets. It documents the general syntax,
available operators and functions and also any specific, non-obvious behaviours. Any differences between Milkdrop's
original parser and projectM's are also documented where applicable.

This document does _not_ cover the `warp_` and `comp_` shader code blocks. The shader code is written in DirectX HLSL,
which is better explained by Microsoft's documentation in the MSDN.

The document also doesn't cover the different code blocks, predefined variables and scope of the special t/q values.
Please refer to the preset authoring guide for these.

## General Syntax

The expression language only knows a few syntactic features:

- Numbers, with or without decimals. Can also contain a base 10 exponent using the `e[+-]n` notation.
- Variables to hold a single number, both user-defined and predefined. May contain characters a-z, 0-9 and the
  underscore (`_`), while 0-9 must not be the first letter.
- Special constants beginning with `$`.
- A single expression, separated from other expressions in the same list via semicolons (`;`), with the last semicolon
  being optional.
- Calls to built-in functions in the form `func(arg1, arg2, ...)`.
- Mathematical, assignment and other operators.
- Subscript access `[]` for megabuf/gmegabuf access.
- Parentheses (`()`) to group statements and override precedence.
- In-line comments starting with `//` until the end of the current line
- Multi-line comments starting with `/*` and ended with `*/`.

### Return Types

Expressions can return either a numeric value or a reference to a variable or memory location. What is actually returned
is determined by the function or operator that is executed last. If the expression only consists or a single token, then
the following applies:

- If the expression token is a constant number (`1.0;`), the number is returned by value.
- If the expression token is a variable name (`x;`), a reference to the variable is returned.

For most functions, the return type doesn't matter. If it's a reference, the value pointed to by the reference is used.
References only behave differently when used on the left side of assignments or in the forst argument of the  `assign()`
function. In this case, the variable or (g)megabuf location the reference points to is assigned a new value.

### Expression Result

Every expression returns its outermost result, as a value or reference as described above. When writing multiple
expressions (separated by semicolon) in a list, the result of the _rightmost_ (or last) expression is returned. The
results of previous expressions are discarded.

In general, each expression will be in the form `var = expression;`, assigning a value to the given variable (or
megabuf index). Expressions will also always return a value, so when used as function arguments, this value will be
used. For example, all the following statements will call the `sin()` function with the argument `4`:

```
x = sin(val = 2 * 2);
y = sin(2 + 2);
z = sin(4.0);
```

### Variable and Function Names, Limits

Variable and function names are generally case-insensitive, making `variable` equivalent to `vArIaBlE`.

Any text in expressions - starting with either a letter from a to z or an underscore, followed by more of these
characters and also numbers from 0 to 9 - is first checked to be a built-in function. If it is not a function, the
parser will interpret it as a variable.

Functions always need to be followed by parentheses (`sin(expr)`), while variables must not.

Milkdrop imposes a 16-character limit on variable names. projectm-eval does not have a hard limit, but it's advisable to
not use more than 16 characters for Milkdrop compatibility.

Milkdrop also has a limit of 64 user-defined variables, which also isn't applied by projectm-eval.

#### Global "Register" Variables

In addition to gmegabuf, there are an additional 100 variables which can be used to store global values. These variables
are named `reg00`, `reg01` etc. up to `reg99`. Values stored in those variables are available in al execution contexts,
in the same way as gmegabuf.

Note the index must always be written with two digits. `reg3` is _not_ considered a global variable and will only have a
local scope as any other variable. The same is true for more digits like `reg123`.

Same as with gmegabuf, global variables are not necessarily `0` when a preset is initialized, and they can change at any
time when two presets using the same global variables are blended during a transition.

### Constants

All constants are converted into floating-point numbers internally, even if written as integers.

As with all other names, constant names (and hexadecimal values) are also case-insensitive.

#### Numerical Constants

Basic numerical/decimal constants can be defined in the code in the following formats:

- Integer values: `1` (or `1.`)
- Decimal values: `1.234` (values between 0 and 1 can be abbreviated with `.234`, which equals `0.234`)
- Base 10 exponential form: `1.234e5` equals `123400` and `1.234e-5` equals `0.00001234`

#### Predefined Constants

A few named convenience constants can be used:

- `$PI`: Equals `3.141592653589793`
- `$E`: Equals `2.71828183`
- `$PHI`: Equals `1.61803399`

#### Hexadecimal Constants

Hexadecimal constants start with `$X`, immediately followed by the hexadecimal numbers.

Example: `$XFF` converts to `255`.

#### Character (Ordinal) Constants

Constants can also convert a single character into its ordinal (ASCII) value using the form `$'c'`, where `c` is the
character to convert.

Example: `$'a'` converts to `97`.

### Operator Precedence

Operator precedence is defined in a similar way as in C, except the `^` operator, which has a different
meaning in Milkdrop presets (pow() instead of binary XOR).

See the [C precedence table on cppreference.com](https://en.cppreference.com/w/c/language/operator_precedence) for
details. The pow operator is applied just after the unary plus/minus operators, having a slightly lower precedence.

### Subscript Access

The subscript operator or "array access operator" has a slightly different meaning than in other programming languages.
The Milkdrop expression syntax only knows a single data type, individual numbers, so there are no arrays.

If any index value is not an integer, it is _rounded_ to the nearest integer after calculating the final index (e.g.
adding index and offset first, then rounding).

The subscript operator is instead used to address memory locations in megabuf and gmegabuf. There are three possible
syntax variants:

#### Global Memory Access

Accessing the gmegabuf is possible by using the special keyword `gmem`, followed by an index in the subscript brackets.
The following statement sets gmegabuf location 10.000 to the current value of `x`:

```
gmem[10000] = x;
```

Any memory index from 0 to 8.388.607 (= 128 * 65536) can be addressed.

#### Local Memory Access

Accessing the current context memory buffer (megabuf) can be done by writing the index before a set of empty brackets.
The following example is analogous to the above, but it sets index 10.000 of the local memory instead:

```
10000[] = x;
```

#### Local Memory Access with Offset

An optional offset can be provided in the brackets, which is simply added to the index on the outside. The following
example will set memory index 10.123 to the value of `x`:

```
10000[123] = x;
```

Both index and offset values can of course be calculated with expressions. So the following expression is valid:

```
if(x > 5,5000,1000)[(sin(y) + 1 * .5) * 1000] = z;
```

### Using Parentheses

Parentheses can be used to specify a specific order in which operations are executed. Expressions inside parentheses are
always evaluated, and the result of the evaluation is then used to evaluate any outside expression.

Inside parentheses, using expression lists is also valid. The result is, as described above, the value of the last
expression in the list. For example, the following expression will use `5` as the return value:

```
x = a * (b + c; d + e; 2 + 3);
```

The first two operations will do nothing, as they don't set a variable or memory value and the results are discarded.

### Comments

For more complex calculations, it might be a good idea to leave some comments in the expressions to describe what's done
in a specific line or block. Comments can also be useful to quickly disable a line or block of code to try out things
when writing a preset.

Comments can be used as end-of-line or block variants:

```
x = sin(5); // This comment is only valid until the line ends
y = sin($PI * .5); /* This comment
spans
multiple
lines and the following expression is executed: */ z = 5;
```

Comments can appear inside any expression, but _not_ within a variable, function name, number or other constant.
Writing `x = var/*comment*/iable;` is invalid (but `x = sin/*comment*/(variable);` is).

## Operators

The following operators can be used in expressions.

### Mathematical Operators

These operators carry out basic mathematical operations.

- `+`: Adds both operands.
- `-`: Subtracts the right operand from the left.
- `*`: Multiplies both operands.
- `/`: Divides the left operand by the right. If the divisor is zero, the result of the operation is `0` instead
  of `Inf`.
- `%`: Converts the operands to integers and returns the remainder of the left by the right operand.
- `^`: Calculates the left operand to the power of the right operand.

### Equality Operators

These operators check for equality or inequality of the operands and return a boolean result:

- `==`: Equal. Returns `1` if both operands are equal, `0` if not.
- `!=`: Not equal. Returns `1` if both operands are not equal, `0` if they are.
- `<`: Below. Returns `1` if the left operand is below the right operand, `0` otherwise.
- `>`: Above. Returns `1` if the left operand is above the right operand, `0` otherwise.
- `<=`: Below or equal. Returns `1` if the left operand is below or equal to the right operand, `0` otherwise.
- `>=`: Above or equal. Returns `1` if the left operand is above or equal to the right operand, `0` otherwise.

### Binary Arithmetic Operators

The following operators allow to carry out bitwise operations on integer representations of the operands:

- `&`: Converts both arguments to integers and calculates the bitwise AND between the resulting operands.
- `|`: Converts both arguments to integers and calculates the bitwise OR between the resulting operands.

### Boolean Operators

Boolean operators only check an operand to be zero or non-zero, then return the according result:

- `&&`: Returns `1` if both operands are non-zero and `0` if any operand is zero.
- `||`: Returns `1` if one or both operands are non-zero and `0` if both operands are zero.
- `!`: Unary operator. Returns `1` if the operand is zero and `0` if the operand is non-zero.
- `a ? b : c`: The ternary operator. Evaluates the `a` operand and evaluates and returns the `b` operand if the
  condition is non-zero. If the condition evaluates to zero, only the `c` operand is evaluated and returned.

### Assignment Operators

Assignment operators can simply assign the result of the right-hand side to a variable or megabuf index, or in case of
compound operators `a OP= b` also carry out an additional operation equivalent to `a = a OP B`. All assignment operators
return a reference to the left operand.

- `=`: Assigns the value of the right operand to the left operand.
- `+=`: Adds the value of the right operand to the left operand and assigns the result to the left operand.
- `-=`: Subtracts the value of the right operand from the left operand and assigns the result to the left operand.
- `*=`: Multiplies the value of the right operand with the left operand and assigns the result to the left operand.
- `/=`: Divides the left operand by the value of the right operand and assigns the result to the left operand. If the
  divisor is zero, the result of the operation is `0` instead of `Inf`.
- `%=`: Calculates the remainder of the rounded integer values of the left operand divided by the right operand and
  assigns the result to the left operand. If the divisor is `0`, the result will also be `0`.
- `^=`: Calculates the left operand to the power of the right operand and assigns the result to the left operand.
- `&=`: Calculates the binary AND of the rounded integer values of the left and right operands and assigns the result to
  the left operand.
- `|=`: Calculates the binary OR of the rounded integer values of the left and right operands and assigns the result to
  the left operand.

### Subscript Operator

The subscript - or index - operator `[]` is a special notation to access megabuf and gmegabuf. If the inside of the
brackets is empty, it is interpreted as a `0`. For more details, see the "Subscript Access" section above.

## Built-in Functions

The following functions are built into the expression parser and can be used in all Milkdrop 2.x presets.

### above(left, right)

Returns `1` if `left` has a larger value than `right`, otherwise `0`.

Same as the `>` operator.

### abs(val)

Returns the absolute (positive) value of `val`.

### acos(val)

Calculates and returns the arc cosine (inverse cosine) of `val`. The angle is interpreted as radians.

### asin(val)

Calculates and returns the arc sine (inverse sine) of `val`. The angle is interpreted as radians.

### assign(dest, val)

Assign `val` to the reference pointed by `dest` and return `dest`. If `dest` is not a reference, the result is the
_value_ of `val`.

Same as the assignment operator `=`.

### atan(val)

Calculates and returns the arc tangent (inverse tangent) of `val`. The angle is interpreted as radians.

### atan2(x, y)

Calculates and returns the polar coordinate angle associated with the coordinate represented by `x,y`. The angle is
returned as radians.

See the [Wikipedia definition of atan2](https://en.wikipedia.org/wiki/Atan2) for details.

### band(left, right)

Evaluates both arguments and returns `1` if both arguments evaluate to a non-zero value and `0` if at least one argument
is zero.

Note: The boolean AND operator `&&` returns the same result, but only evaluates the second argument if the first one is
not zero!

### below(left, right)

Returns `1` if `left` has a smaller value than `right`, otherwise `0`.

Same as the `<` operator.

### bnot(val)

Boolean NOT operator. Returns `1` if `val` is `0`, otherwise `0`.

Same as the `!` operator.

### bor(left, right)

Evaluates both arguments and returns `1` if at least one argument evaluates to a non-zero value and `0` if both
arguments are zero.

Note: The boolean OR operator `||` returns the same result, but only evaluates the second argument if the first one is
zero!

### ceil(val)

Rounds the value of `val` to the next _larger_ integer number in reference to `0`.

### cos(val)

Calculates and returns the cosine of `val`. The angle is interpreted as radians.

### equal(left, right)

Compares both values and returns `1` of they are equal or `0`if not.

Same as the `==` operator.

### exec2(expr1, expr2)

Executes both expressions and returns the result of the second one.

### exec3(expr1, expr2, expr3)

Executes all three expressions in order and returns the result of the third one.

### exp(val)

Exponential function with base `e` to the power of `val`.

Equivalent to the expression `$E ^ val`.

### floor(val)

Rounds the value of `val` to the next _smaller_ integer number in reference to `0`.

### freembuf(index)

Requests freeing all memory blocks in `megabuf` (NOT `gmegabuf`) beginning with the block `index` is in. This may result
in memory being freed before the given index.

**Note:** This function is currently a no-op in both Milkdrop and projectM. It does _not_ actually free any memory or
reset the memory contents.

Memory blocks are 65536 entries large, and there is a total of 128 blocks.

### gmegabuf(index)

Returns a reference to the storage location of `index` in the global memory buffer.

Any memory index from 0 to 8.388.607 (= 128 * 65536) can be addressed. Any index outside of this range will return the
fixed _value_ `0`, and no memory is changed when assigning another value to it.

### if(cond, trueval, falseval)

Evaluates `cond`, and returns `trueval` if the result is non-zero. Otherwise, `falseval` is returned.

If either `trueval` or `falseval` evaluate to a reference, the reference is passed through. Thus, assigning a value to
the result of `if()` is possible:

`if(x > 5, a, b) = 10;` will assign `10` to `a`  if `x > 5` and to `b` if `x <= 5`.

### int(val)

Truncates any decimals from `val`, effectively converting it into an integer.

### invsqrt(val)

Calculates the [fast inverse square root](https://en.wikipedia.org/wiki/Fast_inverse_square_root) of `val`.

### log(val)

Calculates and returns the ("natural") logarithm to base `e` of `val`.

### log10

Calculates and returns the logarithm to base 10 of `val`.

### loop(count, expr)

Executes `expr` exactly `count` times.

As mentioned in the grammar section, `expr` can also consist of multiple expressions separated by semicolon. this is
also true for `count`, but expressions in this argument are only evaluated once before the first loop iteration starts.

The loop will always abort after 2^20 (~1 million) iterations.

Be careful when nesting loops, as the loop count and associated run time will grow exponentially with each nesting
level, which can hang up the application.

### max(val1, val2)

Returns the larger of the two _values_ of `val1` and `val2`.

### megabuf(index)

Returns a reference to the storage location of `index` in the local memory buffer.

Any memory index from 0 to 8.388.607 (= 128 * 65536) can be addressed. Any index outside of this range will return the
fixed _value_ `0`, and no memory is changed when assigning another value to it.

### memcpy(dest, src, count)

Copies `count` values starting at `src` to `dest`. Returns a reference to `dest`.

Memory areas are allowed to overlap.

Any source or destination memory falls outside the allowed indices of 0 to 8.388.607, the range is trimmed accordingly
and only a data block is copied that fits into both areas.

### memset(dest, value, count)

Fills `count` number of memory slots beginning at `dest` with `value`.

If the range falls outside the allowed indices of 0 to 8.388.607, the range is trimmed accordingly.

This function is _way_ faster than setting individual values to 0 in a `loop()` and should be preferred if a larger
memory block needs to be initialized or reset with a single number.

### min(val1, val2)

Returns the smaller of the two _values_ of `val1` and `val2`.

### pow(base, exp)

Calculates `base` to the power of `exp` and returns the resulting value.

### rand(max)

Calculates a pseudo-random number (using the Mersenne-Twister algorithm) between 0 and `max`.

`max` can be a floating-point value.

### sigmoid(a, b)

Calculates the logistic curve with an argument of `(-a * b)` and returns the result.

This is equivalent to the expression `tmp = 1 + pow($E, (-a + b)); if(tmp > 0.00001, 1 / tmp, 0);`.

### sign(val)

Returns the signage of `val`, which is `-1` for negative values, `1` for positive values and `0` for `0`.

### sin

Calculates and returns the sine of `val`. The angle is interpreted as radians.

### sqr(val)

Calculates and returns the square of `val`.

This is equivalent to the expression `val * val`.

### sqrt(val)

Calculates and returns the square root of `val`.

### tan(val)

Calculates and returns the tangent of `val`. The angle is interpreted as radians.

### while(expr)

Executes `expr` at least once and loops until it returns `0`.

`expr` can be a semicolon-separated list of expressions. Only the return value of the last expression in the list is
used to determine the loop abort condition.

The loop will always abort after 2^20 (~1 million) iterations.

Be careful when nesting loops, as the loop count and associated run time will grow exponentially with each nesting
level, which can hang up the application.

### Internal Functions and Aliases

The following functions are defined internally, some being aliases to the above functions, others being used as actual
implementations of operators. Depending on the operator being unary or binary, each function has one or two parameters.

- _aboeq => Operator `>=`
- _above => Operator `>`
- _add => Operator `+`
- _addop => Operator `+=`
- _and => Operator `&&`
- _andop => Operator `&=`
- _beleq => Operator `<=`
- _below => Operator `<`
- _div => Operator `/`
- _divop => Operator `/=`
- _equal => Operator `==`
- _gmem => Alias for `gmegabuf`
- _if => Operator `?:`, alias for `if`
- _mem => Alias for `megabuf`
- _mod => Operator `%`
- _modop => Operator `%=`
- _mul => Operator `*`
- _mulop => Operator `*=`
- _neg => Negation operator `-`
- _not => Operator `!`
- _noteq => Operator `!=`
- _or => Operator `||`
- _orop => Operator `|=`
- _powop => Operator `^=`
- _set => Operator `=`, alias for `assign`
- _sub => Operator `-`
- _subop => Operator `-=`

### Externally-defined Functions

Be aware that if this parser is used in other projects or non-Milkdrop-compatible presets, additional functions may be
defined which can't be used as variable names. One example are the additional `getosc` and `getspec` functions defined
in AVS.

## Memory Buffers

Each execution context has two associated memory buffers, a local buffer and a global buffer. Each buffer can hold up to
2^23 (8.388.608) individual floating-point numbers which can be addressed via their zero-based index.

The local buffer is only valid within the same execution context. In classic Milkdrop, there are these contexts, each of
which having its own `megabuf`:

- The global per-frame context (`per_frame_init_` and `per_frame_`)
- The per-vertex/per-pixel mesh context (`per_pixel_`)
- The wave per-frame context, one per waveform (`wave_N_init` and `wave_N_per_frame`)
- The wave per-point context, one per waveform (`wave_N_per_point`)
- The shape per-frame context, one per shape (`shape_N_init` and `shape_N_per_frame`)

The global memory buffer, referred to as `gmegabuf`, only exists once and is used for _all_ execution contexts. It is
even shared between different Milkdrop presets. Keep this in mind when using the buffer, as the contents may be changed
by another preset during the transition phase!

Local buffers always start empty and return zero for all indices when being accessed the first time. The global buffer
will also start this way, but is never reset. Code using the global megabuf should initialize the required indices with
known values.

Memory is allocated in blocks of 64K (65.536) values each, with a total of 128 of such blocks per buffer. When using
memory indices, ideally keep the indices close together to allocate only a minimum number of blocks and save the user's
RAM resources.

### Setting Large Blocks of Memory to a Certain Value

If a larger memory region needs to be initialized with a single value, e.g. resetting a part of `gmegabuf`, assigning
single values in a `loop` is time-consuming. The `memset` function can be used to achieve the same result in a very fast
operation. Calling the following function will reset values 10.000 to 19.999 to `5`:

```
memset(10000, 5, 10000);
```

### Copying/Moving a Range of Data

Sometimes it's required to copy or move a continuous block of values, e.g. moving data from previous frames left and
adding a new set of data points at the end. This can be done in a `loop`, but again is very time-consuming. The `memcpy`
function is able to perform this operation in a single call. It also supports moving memory in overlapping areas - the
original contents are first copied into a temporary buffer, then written to the new location. To move data forward in a
1000-value large block by 100 indices, with the block starting at index 5000 looks like this:

```
memcpy(5000, 5100, 900);
```

The last 100 values will appear twice in the block, while the first 100 got overwritten.

## Caveats and Tricks

For most uses, the expression language is quite straightforward to use. Due to the implementation, users can take
advantage of some features while they need to be aware of a few details that may cause problems.

### Using References

As mentioned in the sections above, some functions may or may not return variable or memory references. It is, in any
case, valid to assign a value to any other value, even a constant. So while the following statement doesn't do anything
useful, it is valid:

```
assign(1, 2);
```

The function itself will return the value `2` as expected, but the actual assignment is discarded.

Other constructs are more helpful, for example to choose between two or more variables to assign a value to. The
following code will assign `10` to either `a`, `b` or `c` depending on the value of `x`:

```
assign(if(x < 0, a, if(x > 0), c, b), 10);
```

This works because `if` will pass the result of the appropriate true/false expression, which can be a reference to a
variable. In the above example, it would also be valid to return `0` instead of `b` is no assignment is wanted, but
the `assign()` function should still return the value 10 in any case.

The last example can also be written in the assignment form, with `if()` as the left operand:

```
if(x < 0, a, if(x > 0), c, b) = 10;
```

### Conditions in `if` and `while`

A few functions check a condition to execute a certain instruction. For example, `if` decides whether to use the true or
false argument, and `while` uses the value of the last expression to check if the loop should abort.

Due to how floating-point numbers work in computers, not all calculations may return exactly `0.0`, even when using
simple constants. The reason is that some numbers can't be exactly represented by the binary number used by the CPU and
needs to be slightly modified. Using such a number in math equations may even increase the error, and the result might
never be exactly zero.

To make the expression work as intended, one of the following approaches may be used, depending on what is required:

1. If only the integer part of the value is of interest, use `int(val)` to truncate any decimals.
2. Round the result to the nearest integer with `round(val)` if the error can be larger than `0.5`.
3. Compare the absolute value of the number to be smaller/larger than the allowed error, e.g. `abs(val) > 0.0001`

The boolean comparison operators (`<`, `==` etc.) will always return either `0.0` or `1.0`, which don't need to be
rounded and can be used directly for the condition check. Using those operators should always be preferred over directly
passing a calculated variable value as the conditional.

### Performance

While execution of the compiled programs is generally quite fast, some functions and code constructs can heavily impact
the performance, especially on low-end and embedded devices like the Raspberry Pi. To make presets available to a broad
audience, keep the following considerations in mind:

- Keep code in the `per_pixel_` block small and fast. If possible, perform CPU-intensive work in the `per_frame_` block
  and store the result in the `q` variables (or `gmegabuf` if required).
- The same is true for instanced shape and waveform `per_point` code.
- Do not overuse `loop` and `while`, especially avoid nesting them. If they are required, make sure the abort condition
  on the `while` loop is not affected by rounding errors. Try not to calculate heavy stuff inside a loop.
- Some math functions, specifically trigonometrical ones like `sin` and also `sqrt`, are expensive. Try to cache and
  reuse values instead of recalculating them multiple times.
- Try not to use `megabuf` and `gmegabuf` for everything. Normal variables, including the global `regNN`, have less
  overhead, as they do not require the additional memory allocation checking and address calculation.
