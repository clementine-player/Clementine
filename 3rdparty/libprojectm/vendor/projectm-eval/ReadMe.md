projectM Expression Evaluation Library
======================================

This repository contains the source code for the projectM Expression Evaluation Library, short "projectm-eval", which is
a cross-platform and cross-architecture reimplementation of Milkdrop's pendant, the NullSoft Expression Evaluation
Library 2, or short "ns-eel2".

This library aims at being as portable as possible, sacrificing some performance over portability. The original ns-eel2
library directly assembles machine code from compiled assembler fragments. While this is super performant, the assembler
code needs to be rewritten manually for every target platform and CPU architecture and is really hard to read or edit.

projectM's Expression Evaluation Library was implemented as a separate project and put under the MIT license to make it
useful in other projects, open source and closed source, while the core projectM library stays under the LGPL license.

Implementation and usage details can be found in the ["docs" subdirectory](docs).

## Building the Library

The projectM Expression Evaluation Library is always being built as a static library. It can also be used as an object
library in a subproject of other CMake projects, see the instructions below on how to use it this way.

### Requirements

If you just want to build the library, no other dependencies besides a working toolchain, CMake (3.20 or higher) and a
build processor like Make, Ninja, Visual Studio or Xcode are required.

To recreate the parser and lexer or to run tests, additional dependencies need to be installed:

- Bison 3.8 and Flex 2.6 to regenerate the code compiler from the .y/.l files.
- GTest 1.10 or higher to run the unit tests
- Google Benchmark to run the benchmarks.

To specify custom locations for Bison and Flex, set BISON_EXECUTABLE and FLEX_EXECUTABLE accordingly when configuring
the project.

### Configure and Build

For a default build, the library doesn't need any additional specifiers other than the standard CMake parameters like
source dir, build dir and generator. Execute the following commands from the source directory (where this file resides):

```shell
mkdir cmake-build
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
cmake --build cmake-build
cmake --install cmake-build
```

The resulting files can then be used in other projects. See the Quick Start Guide below for details.

## Quick Start Guide

The following guide gives a short overview on what is needed to get your first script running.

If you need to use the ns-eel2 API, please refer to the [NullSoft shim ReadMe](ns-eel2-shim/ReadMe.md) instead.

### Using the Library with CMake

#### As an External Project (Static Library)

If you want to use projectM-Eval as an external project in your CMake builds, use `find_package()` in your project's
CMakeLists.txt:

```cmake
find_package(projectM-Eval REQUIRED)
```

You may need to point CMake to the installation directory via
the [`CMAKE_PREFIX_PATH` variable](https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html).

To add the include dir and link the static library, simply link the `projectM::Eval` target to your target, application
or library:

```cmake
add_executable(MyApp
    # Sources go here
)
target_link_libraries(MyApp
    PRIVATE
    projectM::Eval
)
```

Important: If your final target is a static library, this will _not_ copy any code into it! Any application using this
static library will also have to link the `projectM::Eval` static library. To include the projectM-Eval object code into
another static library, use the object library/subproject approach below.

#### As a Subproject (Static or Object Library)

If projectM-Eval is an integral part of your project, adding it as a subproject inside the source tree may be the best
approach, as it will reduce the number of external dependencies and makes building easier.

If possible, using git submodules is highly recommended as it makes updating the sources from upstream easier. Copying
the code into the source tree is also fine though.

Given the sources are checked out/copied to a subdirectory named "projectm-eval" of the directory with the current
CMakeLists.txt, simply adding the subdirectory will suffice:

```cmake
add_subdirectory(projectm-eval)
```

After this include directive, the `projectM::Eval` target is available for linking, same as with the external package
variant. To link the library as a static library, the CMake code is identical:

```cmake
add_executable(MyApp
    # Sources go here
)
target_link_libraries(MyApp
    PRIVATE
    projectM::Eval
)
```

If the target is a static library, to which the projectM-Eval object files should be added, then an additional line in
the target source list is required:

```cmake
add_library(MyLib STATIC
    $<TARGET_OBJECTS:projectM::Eval>
    # Other sources go here
)
target_link_libraries(MyApp
    PRIVATE
    projectM::Eval
)
```

Note that the library still needs to be linked. While this won't add any code to the target, it will populate the
required include directories and compiler flags needed to use the headers.

#### Using the Library with Other Build Systems

If CMake is not an option, the static library can be linked manually and the `projectm-eval.h` header file can be copied
into the project or pointed to via the include directory settings of the build system used.

On UNIX operating systems such as Linux, macOS or BSD and if `ENABLE_PROJECTM_EVAL_INSTALL` is enabled, a
`projectm-eval.pc` pkg-config file will be installed into the destination library dir and can be used by any
pkg-config-compatible build system to link the static library (using `pkg-config --libs projectm-eval`) and retrieve the
correct include dir (using `pkg-config --cflags projectm-eval`).

To use it as an object library, please refer to the platform build tools on how to unpack the static library into its
object files or how to copy its contents into another static library.

### Running the First Code

To integrate projectM-Eval into another application, only a few steps are required to get things set up:

- Implement the memory locking callbacks.
- Create an execution context.
- Compile some code.
- Register and set variables.
- Run the code.
- Destroy the code and context.

Optionally, custom global memory handling can be used. Please see the [memory handling docs](docs/Memory-Handling.md)
for details.

In production code, always check returned pointers before using them!

In most cases, multi-threading will not be used, so the memory stubs can be empty functions. Either add a separate code
file, or put the following code into any existing implementation file:

```c
#include <projectm-eval.h>

void projectm_eval_memory_host_lock_mutex() {}
void projectm_eval_memory_host_unlock_mutex() {}
```

In C, including the header is optional. If the file is a C++ file, either including the header or adding `extern "C"{}`
around the implementations is mandatory to prevent the compiler from performing C++ name mangling on the functions.

To run any code, an execution context is required. It will maintain the variables and megabuf data, while also giving
access to the reg variables and gmegabuf as needed. In this example, we'll use the internal global memory structures for
simplicity, thus we create the context by passing NULL to both parameters.

```c
#include <projectm-eval.h>

#include <stdio.h>

int main()
{
    struct projectm_eval_context* ctx = projectm_eval_context_create(NULL, NULL);
    /* Insert additional code here */
    
    return 0;
}
```

Now that the context is created, we can already compile code:

```c
struct projectm_eval_code* code = projectm_eval_code_compile(ctx, "x = a; y = 2;");
```

If the code couldn't be compiled, NULL is returned. The `projectm_eval_get_error()` function can be used to retrieve the
parser error, including line and column.

The above code will surely compile, but uses a variable `a` which isn't set explicitly. If that's the case, any variable
that was never set before will have an initial value of `0`. Yet in most cases, expressions will run on some input from
the application, so in this example, `a` would be the input. To pass a value to the code, we can register the variable
and use the returned pointer to set or read the value. In this example, we register all three variables and set `a`
to `100` initially:

```c
PRJM_EVAL_F* var_a = projectm_eval_context_register_variable(ctx, "a");
PRJM_EVAL_F* var_x = projectm_eval_context_register_variable(ctx, "x");
PRJM_EVAL_F* var_y = projectm_eval_context_register_variable(ctx, "y");

*var_a = 100.0;
```

The internal `reg00` to `reg99` variables can be registered in the same way.

The megabuf and gmegabuf contents cannot be accessed from the outside. If that is required, compile an expression that
copies it into a normal variable, register the variable, then execute the code and read the variable contents.

Now the code can be executed:

```c
projectm_eval_code_execute(code);
```

Note that only the code handle is required. The context is stored inside this opaque structure, as any compiled code is
firmly tied to a single context. This said, executing code after destroying the context it was compiled in _will_ crash
the application!

In this example, the return value of `projectm_eval_code_execute()` is ignored. If your application is not interested in
variables, but the result of the _last_ statement in the expression, this is the value returned by the function. In this
example, it would be `2`, as this is the result of the `y = 2` expression.

The execution has changed the contents of the `x` and `y` variables, so we can now output all three values to see their
actual contents:

```c
printf("a = %f\nx = %f\ny = %f\n", *var_a, *var_x, *var_y);
```

After we're done with the context and code, everything should be cleaned up properly:

```c
projectm_eval_code_destroy(code);
projectm_eval_context_destroy(ctx);
```

A few notes on cleanup:

- The order in which the context, code handles and memory buffers are destroyed doesn't matter.
- After destroying a handle, do _not_ use this handle or any associated data, e.g. registered variables after the
  context was destroyed. This will crash the application.
- Code handles can be destroyed right after execution. As variables are stored in the context, their values remain
  unchanged as long as the context isn't destroyed.
- _Never_ call `free()` on the registered variable pointers. The memory they point to is owned and freed by the context.

## Further Reading

The [`docs`](docs) directory contains a few more documents regarding the API and expression syntax:

- [Compiler Internals](docs/Compiler-Internals.md): A few technical details on how the projectM-Eval compiler/parser
  works internally.
- [Expression Syntax](docs/Expression-Syntax.md): Full documentation on the expression code syntax, including tricks and
  caveats. This document is also valid for the original Milkdrop parser.
- [Memory Handling](docs/Memory-Handling.md): Additional information for using global memory buffers and share them
  between contexts, plus multi-threading considerations.

If there are still open questions, feel free to visit us on our Discord server. An invitation link can be found in
the [libprojectM repository on GitHub](https://github.com/projectM-visualizer/projectm).

For bugs and feature requests, feel free to contribute a pull request or open an issue in the bug tracker.