Handling Global Memory
======================

Each projectM-Eval context has its own megabuf, which will live inside the context and is deleted when the context is
destroyed. The same is true for any script-defined or externally registered variables. To share global data between
different execution contexts, there are two methods:

- The reg00-reg99 variables.
- The gmegabuf memory buffer.

The reg variables were available for a long time, while megabuf and gmegabuf were introduced in ns-eel2 during Milkdrop
2 development.

## Using the Built-in Global Memory Buffer

If the embedding application doesn't need to use multiple global memory buffers, for example if all execution contexts
will share the same global memory, then the built-in buffer and reg vars can be used by passing NULL for both parameters
in the `projectm_eval_context_create()` function:

```c
struct projectm_eval_context* ctx = projectm_eval_context_create(NULL, NULL);
```

To free and reset the global memory buffer, call `projectm_eval_memory_global_destroy()`. This will not reset the reg
variables though.

## Using Application-defined Global Memory Buffers

Both the gmegabuf and the reg variables can be instantiated by the embedding application to control which execution
contexts will share the data. Both the buffer and the reg variables can be shared individually.

### Custom gmegabuf

To create a custom global memory buffer, call `projectm_eval_memory_buffer_create()` and store the resulting pointer. It
is an opaque data structure which will receive the gmegabuf data as scripts execute. Then pass the pointer to all newly
created contexts. If the buffer is no longer needed, e.g. all contexts using it were destroyed, remember to also free
the buffer.

Example:

```c
projectm_eval_mem_buffer gmbuf = projectm_eval_memory_buffer_create();
struct projectm_eval_context* ctx = projectm_eval_context_create(gmbuf, NULL);

/* Execute stuff */

projectm_eval_context_destroy(ctx);
projectm_eval_memory_buffer_destroy(gmbuf);
```

Note that when creating the buffer, it doesn't allocate any actual buffer memory. Memory is reserved in small blocks
whenever an index is accessed for which no buffer exists yet.

### Custom reg Variables

Similar to the memory buffer, the reg variables need to be allocated and a pointer passed to the context upon creation.
In contrast to the buffer, the reg variables are stored in a simple array of type PRJM_EVAL_F with the size of 100
entries:

```c
PRJM_EVAL_F reg_vars[100] = {0};
struct projectm_eval_context* ctx = projectm_eval_context_create(NULL, &reg_vars);

/* Execute stuff */

projectm_eval_context_destroy(ctx);
```

## Multi-Threading Considerations

In general, executing code in the same context in multiple threads at the same time is generally unsupported as the
variables are shared between contexts. This also holds true for contexts sharing reg variables - while it may work,
multiple scripts changing reg vars in different threads may have negative impact on the other instances.

The above stays true for gmegabuf as well, but allocating memory makes things a bit more complicated. If there is a real
requirement of using the same gmegabuf in multiple threads at once, this can be done by implementing
the `projectm_eval_memory_host_lock_mutex()` and `projectm_eval_memory_host_unlock_mutex()` functions to set and release
proper locks. A non-recursive mutex is sufficient.

Note that using a mutex will prevent race conditions and memory loss (e.g. two thread trying to allocate the same memory
area), but it won't change the unpredictable behaviour of values changing unexpectedly.

As noted in the quick-start guide, an application using projectM-Eval is _required_ to implement the above functions. If
no locking is needed, they can be empty stubs.