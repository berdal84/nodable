#pragma once
#include <cstdlib>
#include <cstring> // for memset
#include "MACROS.hpp"
#include "Types.hpp"

#ifdef BDC_DEBUG_ALLOCATORS
    #include "Stacktrace.hpp"
    #define BDC_LOG_DEBUG_STACKTRACE_WITH_REASON( fmt, ... ) \
        BDC_LOG_STACKTRACE(); \
        BDC_LOG_DEBUG( fmt " (see stacktrace above).\n\n", __VA_ARGS__ );
#else
    #define BDC_LOG_STACKTRACE_WITH_REASON( fmt, ... ) /* BDC_LOG_STACKTRACE_WITH_REASON is disabled */
#endif // BDC_ENABLE_LOGS

#define BDC_DEFAULT_TEMP_BUFFER_SIZE 5 * 1024 * 1024

namespace bdc
{
    struct Memory_Allocation_Info
    {
        void*  data;
        size_t size;
    };

    struct Memory_Manager_Report
    {
        const char*             name;
        bool                    has_leaked;

        struct {
            Memory_Allocation_Info* data;
            size_t                  size;
        } allocations;
    };

    struct Allocator
    {
        using Malloc_Proc_Type  = void* (size_t size);
        using Free_Proc_Type    = void  (void*  ptr );
        using Realloc_Proc_Type = void* (void*  ptr, size_t size);

        const char*        name;
        Malloc_Proc_Type*  proc_malloc;
        Free_Proc_Type*    proc_free;
        Realloc_Proc_Type* proc_realloc;
    };


    struct Allocation_Header
    {
        u64_t size;

        #ifdef BDC_DEBUG_ALLOCATORS
            i64_t owners;
        #endif
    };

    struct Ring_Buffer
    {
        char*               data;
        size_t              size;
        char*               head;
        Allocation_Header*  prev_acquired; // usefull in case realloc just after a malloc, we can keep the same adress since there is nothing after that point.
    };
    extern Allocator*                   allocator; // The current allocator    
    extern Allocator                    temp_allocator;
    extern Ring_Buffer                  temp_allocator_buffer;
    extern Allocator                    heap_allocator;

    void                                memory_manager_init(size_t temp_buffer_size = BDC_DEFAULT_TEMP_BUFFER_SIZE );
    void                                memory_manager_clear_trackers();
    void                                memory_manager_shutdown();
    Memory_Manager_Report*              memory_manager_generate_report(Memory_Manager_Report* report = nullptr);

    struct Report_Options
    {
        bool asserts_no_leaks = true;
    };

    void                                memory_manager_report_print(Memory_Manager_Report* report, Report_Options = {});
    size_t                              memory_manager_reset_temp_allocator_buffer();
    void                                push_allocator(Allocator*);
    void                                pop_allocator();
    
    [[nodiscard]] inline void* memory_malloc(size_t size, Allocator* _allocator = allocator )
    {
        void* ptr = _allocator->proc_malloc( size );
        BDC_ASSERT(ptr != nullptr, "proc_malloc was unable to return memory address");
        BDC_LOG_DEBUG_STACKTRACE_WITH_REASON( "Allocated address {}", ptr );
        return ptr;
    }

    inline void memory_free(void* ptr, Allocator* _allocator = allocator )
    {
        BDC_LOG_DEBUG_STACKTRACE_WITH_REASON( "Freeing address {}", ptr );
        return _allocator->proc_free( ptr );
    }

    [[nodiscard]] inline void* memory_realloc(void* ptr, size_t size, Allocator* _allocator = allocator )
    {
        BDC_ASSERT(ptr != nullptr, "Cannot reallocate a nullptr!");
        BDC_LOG_DEBUG_STACKTRACE_WITH_REASON( "Reallocating address {}", ptr );
        auto new_ptr = _allocator->proc_realloc(ptr, size);
        BDC_ASSERT(new_ptr != nullptr, "Unable to allocate!");
        return new_ptr;
    }

    template<typename Type>
    inline void memory_reset(auto* ptr, int value = 0)
    {
        memset(static_cast<void*>(ptr), value, sizeof(Type));
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_malloc(Allocator* _allocator = allocator )
    {
        return reinterpret_cast<Type*>( memory_malloc( sizeof(Type), _allocator ));
    }
    
    template<typename Type>
    [[nodiscard]] inline Type* memory_realloc( Type* ptr, Allocator* _allocator = allocator )
    {
        return reinterpret_cast<Type*>(_allocator->proc_realloc( ptr, sizeof(Type) ));
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_malloc_array( size_t elem_count , Allocator* _allocator = allocator )
    {
        return reinterpret_cast<Type*>( memory_malloc( sizeof(Type) * elem_count, _allocator ) );
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_realloc_array( Type* ptr, size_t elem_count, Allocator* _allocator = allocator )
    {
        return reinterpret_cast<Type*>( memory_realloc( ptr, sizeof(Type) * elem_count, _allocator ) );
    }

    template<typename Type>
    [[nodiscard]] inline auto* memory_new(Allocator* _new_allocator, auto&&... args )
    {
        Type* ptr = memory_malloc<Type>( _new_allocator );
        new (ptr) Type(std::forward<decltype(args)>(args)...);
        return ptr;
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_new(Allocator* _new_allocator = allocator )
    {
        Type* ptr = memory_malloc<Type>( _new_allocator );
        new (ptr) Type();
        return ptr;
    }

    template<typename Type>
    inline void memory_delete(Type* ptr, Allocator* _allocator = allocator )
    {
        ptr->~Type();
        memory_free( ptr, _allocator );
    }

    template<typename Type>
    bool is_zero_initialized(const Type& obj)
    {
        constexpr size_t size = sizeof(Type);
        const char* bytes = reinterpret_cast<const char*>(&obj);
        for (size_t i = 0; i < size; ++i)
            if (bytes[i] != 0)
                return false;
        return true;
    };

} // namespace bdc
