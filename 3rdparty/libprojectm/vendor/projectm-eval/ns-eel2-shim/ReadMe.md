NullSoft Expression Evaluation Library Shim
===========================================

This directory contains a lightweight API wrapper which makes the projectM Expression Evaluation Library compatible
with Milkdrop, in a way that it can be used as a drop-in replacement for the "ns-eel2" subdirectory in the codebase.

Here's a short overview of what the shim provides, what is missing and which differences to expect:

## Compatibility

### Mapped API Functions

The following API functions are drop-in compatible with the original `ns-eel2.h` header and the projectM functions they
map to, if any:

- NSEEL_HOSTSTUB_EnterMutex -> _prjm_eval_memory_host_lock_mutex_
- NSEEL_HOSTSTUB_LeaveMutex -> _prjm_eval_memory_host_unlock_mutex_
- NSEEL_quit -> _prjm_eval_memory_global_destroy_
- NSEEL_VM_alloc -> _pjrm_eval_context_create_
- NSEEL_VM_free -> _pjrm_eval_context_destroy_
- NSEEL_VM_resetvars -> _pjrm_eval_context_reset_variables_
- NSEEL_VM_regvar -> _pjrm_eval_context_register_variable_
- NSEEL_VM_freeRAM -> _pjrm_eval_context_free_memory_
- NSEEL_VM_SetGRAM -> _(none)_
- NSEEL_VM_FreeGRAM -> _prjm_eval_memory_buffer_destroy_
- NSEEL_code_compile -> _pjrm_eval_code_compile_
- NSEEL_code_getcodeerror -> _prjm_eval_get_error_
- NSEEL_code_execute -> _pjrm_eval_code_execute_
- NSEEL_code_free -> _pjrm_eval_code_destroy_

### Unimplemented API Functions

The following functions are not implemented by the shim, mainly because they're unused in Milkdrop,
provide no useful functionality in ns-eel2 or have no equivalent use case in the projectM pendant:

- NSEEL_init
- NSEEL_addfunction
- NSEEL_addfunctionex
- NSEEL_addfunctionex2
- NSEEL_getstats
- NSEEL_getglobalregs
- NSEEL_VM_enumallvars
- NSEEL_VM_freeRAMIfCodeRequested
- NSEEL_VM_wantfreeRAM
- NSEEL_VM_SetCustomFuncThis
- NSEEL_code_getstats

### Unavailable Defines

The following preprocessor macros are not available, as they only expose some internals of ns-eel2 and aren't useful
outside if it:

- NSEEL_MAX_VARIABLE_NAMELEN
- NSEEL_MAX_TEMPSPACE_ENTRIES
- NSEEL_LOOPFUNC_SUPPORT_MAXLEN
- NSEEL_LOOPFUNC_SUPPORT_MAXLEN_STR
- NSEEL_SHARED_GRAM_SIZE
- NSEEL_RAM_BLOCKS
- NSEEL_RAM_ITEMSPERBLOCK

### Memory Limiting

As of now, megabuf/gmegabuf memory usage limiting is not implemented, as it would require the use of static globals to
keep track of all allocated memory blocks. Milkdrop doesn't limit memory usage, and the provided functions (e.g.
`NSEEL_VM_freeRAMIfCodeRequested`) are not called.

### Handle Compatibility

All handles returned by the shim are compatible with the parameters and return values of the respective mapped
functions, and thus can be used interchangeably.

Note that the handles are _not compatible_ with those provided by ns-eel2 in regard to their actual contents. A handle
created by ns-eel2 cannot be used with the projectM expression evaluation library and vice versa, as the internal data
structures are implemented in completely different ways.

### Memory Locking Functions

There are two functions which can be used to lock or unlock a mutex whenever the library allocates or deallocates
megabuf and gmegabuf blocks. Linking the ns-eel2 shim will implement the `projectm_eval_memory_host_lock_mutex`
and `projectm_eval_memory_host_unlock_mutex` with proxy calls to their respective ns-eel2
pendants, `NSEEL_HOSTSTUB_EnterMutex` and `NSEEL_HOSTSTUB_LeaveMutex`. If the code using this library also defines the
original `prjm_` stubs, it will lead to a "duplicate symbol" linker error.

## Performance and Portability

This shim is aimed at developers who want to port Milkdrop to previously unsupported CPU architectures like ARM. This
means:

1. ns-eel2 will produce faster-running code because it internally uses assembler code, but is only supported on x86 and
   possibly x86_64 Windows machines.
2. projectM-eval will produce lower code, but is portable to basically any CPU architecture and OS as it only uses plain
   C code to implement the actual runtime code.

There are no benchmarks yet, but the projectM code is expected to run slightly slower because of the following reasons:

- More stack frame allocations. One per function/operator call at least. ns-eel2 tries to run the whole code in a single
  large stack frame, only occasionally calling functions.
- Less usage of CPU-specific intrinsics. ns-eel2 uses x86 instructions directly to perform some operations, e.g.
  rounding floats to ints (`fistpl`) or calculating sine (`fsin`) and cosine (`fcos`).

There might be other reasons as well, depending on the actual compiler optimizations, C runtime performance and target
CPU architecture.

## Using Features ns-eel2 doesn't Provide

If full backwards compatibility to ns-eel2 isn't a (strong) requirement, but replacing all existing calls with the
projectM API is not desired, any features the projectM Expression Evaluation Library API provides in addition to the
shim can be used without issues. The shim only provides lightweight wrappers around the projectM API functions, but
doesn't introduce new types. Converting the respective types is safely possible via C-style casts:

```c
NSEEL_VMCTX ctx = NSEEL_VM_alloc();
struct projectm_eval_context* prjm_ctx = (struct projectm_eval_context*)ctx;
```

Analogous, in C++ you should use `reinterpret_cast`:

```c++
NSEEL_VMCTX ctx = NSEEL_VM_alloc();
struct projectm_eval_context* prjm_ctx = reinterpret_cast<struct projectm_eval_context*>(ctx);
```
