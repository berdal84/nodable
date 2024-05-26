#pragma once

#include <cstdlib>
#include <memory>
#include <cstddef>

// When set true, any allocation or deallocation will be logged
#ifndef TOOLS_MEMORY_MANAGER_ENABLE_LOGS
#define TOOLS_MEMORY_MANAGER_ENABLE_LOGS false
#endif

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
