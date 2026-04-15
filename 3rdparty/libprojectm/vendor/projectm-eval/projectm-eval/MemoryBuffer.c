#include "MemoryBuffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PRJM_EVAL_MEM_BLOCKS 128
#define PRJM_EVAL_MEM_ITEMSPERBLOCK 65536

static projectm_eval_mem_buffer static_global_memory;

void prjm_eval_memory_destroy_global()
{
    prjm_eval_memory_destroy_buffer(static_global_memory);
}

projectm_eval_mem_buffer prjm_eval_memory_global()
{
    if (!static_global_memory)
    {
        projectm_eval_memory_host_lock_mutex();

        static_global_memory = prjm_eval_memory_create_buffer();

        projectm_eval_memory_host_unlock_mutex();
    }

    return static_global_memory;
}

projectm_eval_mem_buffer prjm_eval_memory_create_buffer()
{
    return calloc(PRJM_EVAL_MEM_BLOCKS, sizeof(PRJM_EVAL_F*));
}

void prjm_eval_memory_destroy_buffer(projectm_eval_mem_buffer buffer)
{
    prjm_eval_memory_free(buffer);

    free(buffer);
}

void prjm_eval_memory_free(projectm_eval_mem_buffer buffer)
{
    if (!buffer)
    {
        return;
    }

    projectm_eval_memory_host_lock_mutex();

    for (int block = 0; block < PRJM_EVAL_MEM_BLOCKS; ++block)
    {
        if (buffer[block])
        {
            free(buffer[block]);
        }
    }

    memset(buffer, 0, PRJM_EVAL_MEM_BLOCKS * sizeof(PRJM_EVAL_F*));

    projectm_eval_memory_host_unlock_mutex();
}

void prjm_eval_memory_free_block(projectm_eval_mem_buffer buffer, int32_t block)
{
    if (block < 0)
    {
        block = 0;
    }
    if (block < PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK)
    {
        // ns-eel2 sets ctx->ram_needfree to the starting index (plus one) of the first index/block to be freed:
        //    ((INT_PTR *)blocks)[1]=1+d;
        // But since NSEEL_VM_freeRAMIfCodeRequested() is never called, no memory is ever cleared.
    }
}

PRJM_EVAL_F* prjm_eval_memory_allocate(projectm_eval_mem_buffer buffer, int32_t index)
{
    int block;
    if (!buffer)
    {
        return NULL;
    }

    if (index >= 0 && (block = index / PRJM_EVAL_MEM_ITEMSPERBLOCK) < PRJM_EVAL_MEM_BLOCKS)
    {
        PRJM_EVAL_F* cur_block = buffer[block];

        if (!cur_block)
        {
            projectm_eval_memory_host_lock_mutex();

            if (!(cur_block = buffer[block]))
            {
                cur_block = buffer[block] = calloc(sizeof(PRJM_EVAL_F), PRJM_EVAL_MEM_ITEMSPERBLOCK);
            }
            if (!cur_block)
            {
                index = 0;
            }

            projectm_eval_memory_host_unlock_mutex();
        }

        return cur_block + (index & (PRJM_EVAL_MEM_ITEMSPERBLOCK - 1));
    }

    return NULL;
}

PRJM_EVAL_F* prjm_eval_memory_copy(projectm_eval_mem_buffer buffer,
                                   PRJM_EVAL_F* dest,
                                   PRJM_EVAL_F* src,
                                   PRJM_EVAL_F* len)
{
    // Add 0.0001 to avoid using the wrong index due to tiny float rounding errors.
    int32_t offset_dest = (int32_t) (*dest + 0.0001);
    int32_t offset_src = (int32_t) (*src + 0.0001);
    int32_t count = (int32_t) (*len + 0.0001);

    // Trim to front if an offset is less than zero.
    if (offset_src < 0)
    {
        count += offset_src;
        offset_dest -= offset_src;
        offset_src = 0;
    }
    if (offset_dest < 0)
    {
        count += offset_dest;
        offset_src -= offset_dest;
        offset_dest = 0;
    }

    PRJM_EVAL_F* src_pointer;
    PRJM_EVAL_F* dst_pointer;

    while (count > 0)
    {
        int32_t copy_length = count;

        int32_t max_dst_len = PRJM_EVAL_MEM_ITEMSPERBLOCK - (offset_dest & (PRJM_EVAL_MEM_ITEMSPERBLOCK - 1));
        int32_t max_src_len = PRJM_EVAL_MEM_ITEMSPERBLOCK - (offset_src & (PRJM_EVAL_MEM_ITEMSPERBLOCK - 1));

        if (offset_dest >= PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK ||
            offset_src >= PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK)
        {
            break;
        }

        if (copy_length > max_dst_len)
        {
            copy_length = max_dst_len;
        }
        if (copy_length > max_src_len)
        {
            copy_length = max_src_len;
        }

        if (copy_length < 1)
        {
            break;
        }

        src_pointer = prjm_eval_memory_allocate(buffer, offset_src);
        dst_pointer = prjm_eval_memory_allocate(buffer, offset_dest);
        if (!src_pointer || !dst_pointer)
        {
            break;
        }

        memmove(dst_pointer, src_pointer, sizeof(PRJM_EVAL_F) * copy_length);
        offset_src += copy_length;
        offset_dest += copy_length;
        count -= copy_length;
    }

    return dest;
}

PRJM_EVAL_F* prjm_eval_memory_set(projectm_eval_mem_buffer buffer,
                                  PRJM_EVAL_F* dest,
                                  PRJM_EVAL_F* value,
                                  PRJM_EVAL_F* len)
{
    // Add 0.0001 to avoid using the wrong index due to tiny float rounding errors.
    int32_t offset_dest = (int32_t) (*dest + 0.0001);
    int32_t count = (int32_t) (*len + 0.0001);

    // Trim to front
    if (offset_dest < 0)
    {
        count += offset_dest;
        offset_dest = 0;
    }

    if (offset_dest >= PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK)
    {
        return dest;
    }

    if (offset_dest + count > PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK)
    {
        count = PRJM_EVAL_MEM_BLOCKS * PRJM_EVAL_MEM_ITEMSPERBLOCK - offset_dest;
    }

    if (count < 1)
    {
        return dest;
    }

    PRJM_EVAL_F val = *value;
    while(count > 0)
    {
        PRJM_EVAL_F* block_pointer = prjm_eval_memory_allocate(buffer, offset_dest);
        if (!block_pointer)
        {
            break;
        }

        int32_t block_count = PRJM_EVAL_MEM_ITEMSPERBLOCK - (offset_dest & (PRJM_EVAL_MEM_ITEMSPERBLOCK - 1));
        if (block_count > count)
        {
            block_count = count;
        }

        count -= block_count;
        offset_dest += block_count;

        while (block_count--)
        {
            *block_pointer++ = val;
        }
    }

    return dest;
}