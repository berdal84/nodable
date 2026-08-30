#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring> // for memset
#include "MACROS.hpp"
#include "Types.hpp"

#ifdef BDC_DEBUG_ALLOCATORS
#include <vector>    // to store allocation metadata in a container that is outside 
#include <algorithm> // for std::find
#endif // BDC_DEBUG_ALLOCATORS

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

    #ifdef BDC_DEBUG_ALLOCATORS

    struct Memory_Allocation_Tracker
    {
        Allocator*                          allocator; // The one we track
        std::vector<Memory_Allocation_Info> allocations;

        void                                after_malloc(void* ptr, size_t size);
        const Memory_Allocation_Info*       find_allocation(void* ptr) const;
        void                                after_realloc(void* old_ptr, void* new_ptr, size_t new_size );
        void                                before_free(void* ptr);
    };

    #endif // BDC_DEBUG_ALLOCATORS

    struct Allocation_Header
    {
        u64_t size;
    };

    struct Ring_Buffer
    {
        char*  data;
        size_t size;
        char*  head;
        Allocation_Header* prev_acquired; // usefull in case realloc just after a malloc, we can keep the same adress since there is nothing after that point.
    };

    struct Memory_Manager_Context
    {
        Allocator         temp_allocator;
        Ring_Buffer       temp_allocator_buffer;
        Allocator         heap_allocator;
        Allocator*        default_allocator;

        #ifdef BDC_DEBUG_ALLOCATORS
        Memory_Allocation_Tracker  heap_allocator_tracker;
        Memory_Allocation_Tracker  temp_allocator_tracker;
        #endif
    };
    
    Memory_Manager_Context*     memory_manager_init(size_t temp_buffer_size = 4096);
    void                        memory_manager_clear_trackers();
    void                        memory_manager_shutdown();
    Memory_Manager_Report*      memory_manager_generate_report(Memory_Manager_Report* report = nullptr);
    void                        memory_manager_report_print(Memory_Manager_Report* report, bool asserts_no_leaks = true);
    Memory_Manager_Context*     memory_manager();

    inline Allocator*           temp_allocator()                { return &memory_manager()->temp_allocator;         }
    size_t                      temp_allocator_buffer_reset();
    inline Ring_Buffer&         temp_allocator_buffer()         { return memory_manager()->temp_allocator_buffer;   }
    inline Allocator*           heap_allocator()                { return &memory_manager()->heap_allocator;         }
    inline Allocator*           default_allocator()             { return memory_manager()->default_allocator;       }
    
    #ifdef BDC_DEBUG_ALLOCATORS
    inline Memory_Allocation_Tracker& heap_allocator_tracker() { return memory_manager()->heap_allocator_tracker; }
    inline Memory_Allocation_Tracker& temp_allocator_tracker() { return memory_manager()->temp_allocator_tracker; }
    #endif

    [[nodiscard]] inline void* memory_malloc(size_t size, Allocator* allocator = default_allocator() )
    {
        return allocator->proc_malloc( size );
    }

    inline void memory_free(void* ptr, Allocator* allocator = default_allocator() )
    {
        return allocator->proc_free( ptr );
    }

    [[nodiscard]] inline void* memory_realloc(void* ptr, size_t size, Allocator* allocator = default_allocator() )
    {
        return allocator->proc_realloc(ptr, size);
    }

    template<typename Type>
    inline void memory_reset(auto* ptr, int value = 0)
    {
        memset(static_cast<void*>(ptr), value, sizeof(Type));
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_malloc(Allocator* allocator = default_allocator() )
    {
        return reinterpret_cast<Type*>( memory_malloc( sizeof(Type), allocator ));
    }
    
    template<typename Type>
    [[nodiscard]] inline Type* memory_realloc( Type* ptr, Allocator* allocator = default_allocator() )
    {
        return reinterpret_cast<Type*>(allocator->proc_realloc( ptr, sizeof(Type) ));
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_malloc_array( size_t elem_count , Allocator* allocator = default_allocator() )
    {
        return reinterpret_cast<Type*>( memory_malloc( sizeof(Type) * elem_count, allocator ) );
    }

    template<typename Type>
    [[nodiscard]] inline Type* memory_realloc_array( Type* ptr, size_t elem_count, Allocator* allocator = default_allocator() )
    {
        return reinterpret_cast<Type*>( memory_realloc( ptr, sizeof(Type) * elem_count, allocator ) );
    }

    
    template<typename Type>
    [[nodiscard]] inline Type* memory_new(Allocator* allocator = default_allocator() )
    {
        Type* ptr = memory_malloc<Type>( allocator );
        new (ptr) Type();
        return ptr;
    }

    template<typename Type>
    inline void memory_delete(Type* ptr, Allocator* allocator = default_allocator() )
    {
        ptr->~Type();
        memory_free( ptr, allocator );
    }

    // template<typename Type> inline Type* memory_new_array(size_t elem_count, Allocator* allocator = default_allocator() )
    // {
    //     Type* ptr = memory_malloc<Type>( allocator );
    //     for(size_t i = 0; i < elem_count; ++i)
    //         new (ptr + i) Type();
    //     return ptr;
    // }

    // template<typename Type> inline void memory_delete_array(Type* ptr, size_t elem_count, Allocator* allocator = default_allocator() )
    // {
    //     for(size_t i = 0; i < elem_count; ++i)
    //         (ptr + i)->~Type();
    //     memory_free( ptr, allocator );
    // }

} // namespace bdc
