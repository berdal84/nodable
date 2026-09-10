#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring> // for memset
#include "MACROS.hpp"
#include "Types.hpp"
#include <vector>    // to store allocation metadata in a container that is outside 
#include <algorithm> // for std::find
#include <exception>
#include <iostream>

#ifdef BDC_DEBUG_ALLOCATORS
    #include <stacktrace>
    #define BDC_PRINT_STACKTRACE() \
        std::stacktrace st = std::stacktrace::current(); \
        std::cout << st << std::endl
#else
    #define BDC_PRINT_STACKTRACE() /* BDC_PRINT_STACKTRACE is disabled */
#endif // BDC_DEBUG_ALLOCATORS

#ifdef BDC_ENABLE_LOGS
    #define BDC_LOG_STACKTRACE_WITH_REASON( fmt, ... ) \
        printf("Printing stacktrace because: " fmt "\n", __VA_ARGS__); \
        BDC_PRINT_STACKTRACE();
#else
    #define BDC_LOG_STACKTRACE_WITH_REASON( fmt, ... ) /* BDC_LOG_STACKTRACE_WITH_REASON is disabled */
#endif // BDC_ENABLE_LOGS

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

    struct Memory_Allocation_Tracker
    {
        Allocator*                          allocator; // The one we track
        std::vector<Memory_Allocation_Info> allocations;

        void                                after_malloc(void* ptr, size_t size);
        const Memory_Allocation_Info*       find_allocation(void* ptr) const;
        void                                after_realloc(void* old_ptr, void* new_ptr, size_t new_size );
        void                                before_free(void* ptr);
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
    extern Memory_Allocation_Tracker    temp_allocator_tracker;

    extern Allocator                    heap_allocator;
    extern Memory_Allocation_Tracker    heap_allocator_tracker;

    void                                memory_manager_init(size_t temp_buffer_size = 5 * 1024 * 1024 /* 5M*/);
    void                                memory_manager_clear_trackers();
    void                                memory_manager_shutdown();
    Memory_Manager_Report*              memory_manager_generate_report(Memory_Manager_Report* report = nullptr);
    void                                memory_manager_report_print(Memory_Manager_Report* report, bool asserts_no_leaks = true);
    size_t                              memory_manager_reset_temp_allocator_buffer();
    void                                push_allocator(Allocator&);
    void                                pop_allocator();
    
    [[nodiscard]] inline void* memory_malloc(size_t size, Allocator* _allocator = allocator )
    {
        void* ptr = _allocator->proc_malloc( size );
        BDC_LOG_STACKTRACE_WITH_REASON( "Allocated address %p", ptr );
        return ptr;
    }

    inline void memory_free(void* ptr, Allocator* _allocator = allocator )
    {
        BDC_LOG_STACKTRACE_WITH_REASON( "Freeing address %p", ptr );
        return _allocator->proc_free( ptr );
    }

    [[nodiscard]] inline void* memory_realloc(void* ptr, size_t size, Allocator* _allocator = allocator )
    {
        BDC_LOG_STACKTRACE_WITH_REASON( "Reallocating address %p", ptr );
        return _allocator->proc_realloc(ptr, size);
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
    [[nodiscard]] inline Type* memory_new(Allocator* _allocator = allocator )
    {
        Type* ptr = memory_malloc<Type>( _allocator );
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
