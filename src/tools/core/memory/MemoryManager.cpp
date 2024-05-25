#ifdef TOOLS_DEBUG
#include "MemoryManager.h"
#include "tools/core/reflection/reflection"
#include <new>

using namespace tools;

static MemoryStats* g_memory_stats{ nullptr };

const MemoryStats* tools::get_memory_stats()
{
    return g_memory_stats;
}

const MemoryStats* tools::init_memory_manager()
{
    assert(g_memory_stats == nullptr);
    g_memory_stats = new MemoryStats();
    return g_memory_stats;
}

void tools::shutdown_memory_manager()
{
    assert(g_memory_stats != nullptr);
    delete g_memory_stats;
    g_memory_stats = nullptr;
}

namespace tools
{
    // To insert metadata at the beginning of each allocated area
    struct Header
    {
        size_t size;
    };

    static constexpr size_t HEADER_SIZE = sizeof(Header);

    Header* get_header(void* data)
    {
        auto* header = ((Header*)data) - 1;
        return header;
    }

    void* get_data(Header* header)
    {
        return header + 1;
    }

    void* allocate(std::size_t data_size = 0 ) noexcept(false)
    {
        if( data_size == 0)
        {
            return nullptr;
        }

        // Allocate a space for the header followed by the data.
        // [<- header ->|<------ data -------------------->]
        auto* header = (Header*)malloc( HEADER_SIZE + data_size );
        header->size = data_size;

        if ( g_memory_stats )
        {
            g_memory_stats->alloc_sum++;
            g_memory_stats->mem_alloc_sum += data_size;
        }

        void* data = get_data(header);
        return data;
    }

    void deallocate(void* data, std::size_t size = 0 ) noexcept
    {
        Header* header = get_header(data);

        if ( g_memory_stats )
        {
            g_memory_stats->dealloc_sum++;
            g_memory_stats->freed_mem_sum += header->size;
        }

        free(header);
    }
}

void* operator new(size_t size)
{
    return tools::allocate( size );
}

void* operator new(size_t size, const std::nothrow_t&) noexcept
{
    return tools::allocate( size );
}

void* operator new[](size_t size)
{
    return tools::allocate( size );
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept
{
    return tools::allocate( size );
}

void operator delete(void* ptr)
{
    return tools::deallocate( ptr );
}

void operator delete(void* ptr, size_t size)
{
    return tools::deallocate( ptr, size );
}

void operator delete(void* ptr, size_t size, const std::nothrow_t&) noexcept
{
    return tools::deallocate( ptr, size );
}

void operator delete[](void* ptr, size_t size)
{
    return tools::deallocate( ptr, size );
}

void operator delete[](void* ptr, size_t size, const std::nothrow_t&) noexcept
{
    return tools::deallocate( ptr, size );
}

void operator delete[](void* ptr)
{
    return tools::deallocate( ptr );
}

#endif