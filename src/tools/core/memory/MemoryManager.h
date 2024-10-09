#pragma once

#include <cstdlib>
#include <memory>
#include <cstddef>

#ifndef TOOLS_MEMORY_MANAGER_ENABLED
#define TOOLS_MEMORY_MANAGER_ENABLED TOOLS_DEBUG
#endif

#ifndef TOOLS_MEMORY_MANAGER_VERBOSE_LOGS
#define TOOLS_MEMORY_MANAGER_VERBOSE_LOGS false
#endif

#ifndef TOOLS_DEBUG
#define TOOLS_DEBUG_DISABLED_MESSAGE// This macro is disabled when TOOLS_DEBUG is not defined (usually when not compiling in Debug).
#define try_TOOLS TOOLS_DEBUG_DISABLED_MESSAGE
#define catch_TOOLS TOOLS_DEBUG_DISABLED_MESSAGE
#define try_TOOLS_MAIN TOOLS_DEBUG_DISABLED_MESSAGE
#define catch_TOOLS_MAIN TOOLS_DEBUG_DISABLED_MESSAGE
#else // TOOLS_DEBUG

#define try_TOOLS \
    try

#define catch_TOOLS \
    catch(const cpptrace::logic_error& logic_error) \
    { \
        logic_error.trace().print_with_snippets();  \
        std::cout << std::flush; \
        exit(1); \
    } \
    catch(const std::exception & std_error) \
    { \
        std::cout << std_error.what() << std::flush; \
        exit(1); \
    }

#define try_TOOLS_MAIN \
    tools::init_memory_manager(); \
    try_TOOLS

#define catch_TOOLS_MAIN \
    catch_TOOLS \
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
