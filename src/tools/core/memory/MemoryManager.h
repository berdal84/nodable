#pragma once

#include <cstdlib>
#include <memory>
#include <cstddef>

#ifndef TOOLS_MEMORY_MANAGER_ENABLE_ALLOCATION_DEALLOCATION_LOGS
#define TOOLS_MEMORY_MANAGER_ENABLE_ALLOCATION_DEALLOCATION_LOGS false
#endif

#ifndef TOOLS_DEBUG
#define TOOLS_DEBUG_TRY // Macro disabled when TOOLS_DEBUG is not defined.
#define TOOLS_DEBUG_CATCH // Macro disabled when TOOLS_DEBUG is not defined.
#else // TOOLS_DEBUG
#define TOOLS_DEBUG_TRY     \
    tools::init_memory_manager(); \
try

#define TOOLS_DEBUG_CATCH                               \
    catch(const cpptrace::logic_error& logic_error) \
{ \
    logic_error.trace().print_with_snippets(); \
    exit(1); \
} \
catch(const std::exception & std_error) \
{ \
    std::cout << std_error.what() << std::endl; \
    exit(1); \
} \
tools::shutdown_memory_manager();
#endif // TOOLS_DEBUG

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
