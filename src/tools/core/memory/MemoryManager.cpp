#include "MemoryManager.h"
#include "tools/core/reflection/reflection"
#include <cassert>
#include <cstdio>

using namespace tools;

static MemoryStats* g_memory_stats{ nullptr };

const MemoryStats* tools::get_memory_stats()
{
    return g_memory_stats;
}

const MemoryStats* tools::init_memory_manager()
{
    EXPECT(g_memory_stats == nullptr, "Can't initialize twice");
    g_memory_stats = new MemoryStats();
    return g_memory_stats;
}

void tools::shutdown_memory_manager()
{
    EXPECT(g_memory_stats != nullptr, "Already shutdown or never initialized?");
    delete g_memory_stats;
    g_memory_stats = nullptr;
}

namespace tools
{
    // To insert metadata at the beginning of each allocated area
    struct Header
    {
        static constexpr size_t IN_USE = 0xF0F0F0F0F0F0F0F0;
        static constexpr size_t FREED  = ~IN_USE;
        size_t state{ IN_USE }; // header must ALWAYS start with this
        size_t size;
        char notes[256]{};
        explicit Header(size_t size)
            : size(size)
        {
            memset(notes, 0, 256);
        }
        bool owned() const
        { return state == Header::IN_USE; }
        void set_free()
        { state = FREED; }
    };

    static_assert(sizeof(Header) % 8 == 0);

    // Get a pointer to the Header of a given data ptr
    // Returns nullptr if data was not allocated by us
    inline Header* get_header(void* data)
    {
        auto* header = (Header*)((u8_t*)data - sizeof(Header));
        if( header->owned() )
        {
            return header;
        }
        return nullptr;
    }

    // Get a pointer to the data from a given header
    inline void* get_data(Header* header)
    {
        return (u8_t*)header + sizeof(Header);
    }

    void* allocate(std::size_t size ) noexcept
    {
        size = size == 0 ? 1 : size;

        // Allocate a space for the header followed by the data.
        // [<- header ->|<------ data -------------------->]

        size_t block_size = sizeof(Header) + size;
        auto* header = (Header*)std::malloc( block_size );

        *header = Header{ size };

        void* data = get_data(header);

        if ( g_memory_stats )
        {
            g_memory_stats->alloc_sum++;
            g_memory_stats->mem_alloc_sum += size;
#if TOOLS_MEMORY_MANAGER_ENABLE_ALLOCATION_DEALLOCATION_LOGS
            printf("%p allocate %zu B \n", data, block_size);
            fflush(stdout);
#endif
        }

        return data;
    }

    void deallocate(void* data, size_t size) noexcept
    {
        if( data == nullptr )
        {
            return;
        }

        void* ptr = data;

        // if owned
        if ( Header* header = get_header(data) )
        {
            ptr = header;
            if ( g_memory_stats )
            {
                g_memory_stats->dealloc_sum++;
                g_memory_stats->freed_mem_sum += header->size;
#if TOOLS_MEMORY_MANAGER_ENABLE_ALLOCATION_DEALLOCATION_LOGS
                printf( "%p deallocate %zu B\n", header, header->size );
                fflush( stdout );
#endif
            }
        }

        std::free(ptr);
    }
}

void* operator new(size_t size )
{
    return tools::allocate( size );
}

void* operator new[](size_t size )
{
    return tools::allocate( size );
}

void operator delete (void* ptr) noexcept
{
    return tools::deallocate( ptr, 0 );
}

void operator delete[](void* ptr, size_t size ) noexcept
{
    return tools::deallocate( ptr, size );
}
