#pragma once

#ifdef TOOLS_DEBUG
#include <cstddef>
namespace tools
{
    struct MemoryStats
    {
        size_t alloc_count() const
        { return alloc_sum - dealloc_sum; }

        size_t mem_usage() const
        { return mem_alloc_sum - freed_mem_sum; }

        size_t alloc_sum{0};
        size_t dealloc_sum{0};
        size_t mem_alloc_sum{0};
        size_t freed_mem_sum{0};
    };

    const MemoryStats* init_memory_manager();
    void               shutdown_memory_manager(); // undo init_memory_manager
    const MemoryStats* get_memory_stats(); // call init_ before to use
}
#endif