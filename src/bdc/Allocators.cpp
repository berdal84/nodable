#include "Allocators.hpp"

#ifdef BDC_DEBUG_ALLOCATORS
#include <vector>    // to store allocation metadata in a container that is outside 
#include <algorithm> // for std::find
#endif // BDC_DEBUG_ALLOCATORS

namespace bdc
{
    constexpr static size_t      ALLOCATOR_STACK_CAPACITY = 64;
    static Allocator*            allocator_stack[ALLOCATOR_STACK_CAPACITY];
    static size_t                allocator_stack_size;
    Allocator*                   allocator; // The current allocator    
    Allocator                    temp_allocator;
    Ring_Buffer                  temp_allocator_buffer;
    Memory_Allocation_Tracker    temp_allocator_tracker;
    Allocator                    heap_allocator;
    Memory_Allocation_Tracker    heap_allocator_tracker;

    inline Allocation_Header* get_header(void* fat_pointer)
    {
        if( fat_pointer == nullptr)
        {
            return nullptr;
        }
        return (Allocation_Header*)(((char*)fat_pointer) - sizeof(Allocation_Header::size));
    }

    inline void* get_pointer(Allocation_Header* header)
    {
        if( header == nullptr)
        {
            return nullptr;
        }
        return ((char*)header) + sizeof(Allocation_Header::size);
    }

    Allocation_Header* temp_allocator_buffer_acquire(size_t size)
    {
        if ( size == 0 )
        {
            BDC_LOG("temp_allocator_acquire() - WARNING: zero-sized allocation requested.\n");
            return nullptr;
        }
        
        size_t allocation_size = sizeof(Allocation_Header) + size;
        size_t size_used = (size_t)temp_allocator_buffer.head - (size_t)temp_allocator_buffer.data;

        if ( allocation_size > temp_allocator_buffer.size - size_used )
        {
            BDC_LOG("temp_allocator_acquire() - WARNING: temp_allocator_buffer has not enough space left"
                    " (usage %zu/%zu Bytes) or is too small to allocate %zu Bytes.\n",
                    size_used, temp_allocator_buffer.size, size );
            assert(false && "temp buffer is full!");
        }

        auto ptr = (Allocation_Header*)temp_allocator_buffer.head;
        ptr->size = size;

        temp_allocator_buffer.prev_acquired  = ptr;
        temp_allocator_buffer.head          += allocation_size;

        return ptr;
    }

    size_t memory_manager_reset_temp_allocator_buffer()
    {       
        size_t freed_space                  = temp_allocator_buffer.head - temp_allocator_buffer.data;
        temp_allocator_buffer.head          = temp_allocator_buffer.data;
        temp_allocator_buffer.prev_acquired = nullptr;

        #ifdef BDC_DEBUG_ALLOCATORS
            temp_allocator_tracker.allocations.clear();
        #endif     

        return freed_space;
    }

    void push_allocator(Allocator& _allocator_to_push)
    {
        assert(allocator_stack_size < ALLOCATOR_STACK_CAPACITY);
        allocator_stack[allocator_stack_size] = &_allocator_to_push;
        allocator = &_allocator_to_push;
        allocator_stack_size++;
    }

    void pop_allocator()
    {
        assert(allocator_stack_size > 0);
        allocator_stack_size--;
        allocator = allocator_stack[allocator_stack_size-1];
    }


    void* temp_allocator_malloc(size_t size)
    {
        if (size == 0)
            return nullptr;

        Allocation_Header* header = temp_allocator_buffer_acquire(size);

        #ifdef BDC_DEBUG_ALLOCATORS
            header->owners += 1;
            temp_allocator_tracker.after_malloc(get_pointer(header), size);
        #endif     

        return get_pointer(header);
    }

    void temp_allocator_free(void* ptr)
    {
        BDC_LOG_DEBUG("temp_allocator_release() - nothing to do ...\n");
        
        Allocation_Header* header = get_header(ptr);
        if( header )
        {
            header->size = 0;

            #ifdef BDC_DEBUG_ALLOCATORS
                header->owners -= 1;
                if( header->owners < 0 )
                {
                    BDC_DEBUG_ALLOCATORS_PRINT_STACKTRACE_BECAUSE("Potential double free at %p\n", ptr);
                }
            #endif
        }
    }

    void* temp_allocator_realloc(void* src_ptr, size_t size)
    {
        assert(src_ptr != nullptr);
        
        Allocation_Header* src_header = get_header(src_ptr);
        
        #ifdef BDC_DEBUG_ALLOCATORS
            src_header->owners -= 1;
        #endif

        // When ptr was previously aquired, we can simply extend it
        if ( src_header == temp_allocator_buffer.prev_acquired )
        {
            temp_allocator_buffer.head = (char*)temp_allocator_buffer.prev_acquired;
        }
        
        Allocation_Header* dest_header = temp_allocator_buffer_acquire(size);
        void*              dest_ptr    = get_pointer(dest_header);
        assert(dest_ptr && "Unable to acquire memory from temp buffer is nullptr!");

        if( src_ptr != dest_ptr )
        {
            memcpy( dest_ptr, src_ptr, src_header->size );
        }

        #ifdef BDC_DEBUG_ALLOCATORS
            dest_header->owners += 1;
            temp_allocator_tracker.after_realloc(src_ptr, dest_ptr, size);
        #endif

        return dest_ptr;
    }

    void* heap_allocator_malloc(size_t size)
    {
        void* ptr = std::malloc( size );

        #ifdef BDC_DEBUG_ALLOCATORS
            assert(ptr != nullptr);
            heap_allocator_tracker.after_malloc(ptr, size);
        #endif
        
        return ptr;
    }

    void heap_allocator_free(void* ptr)
    {
        #ifdef BDC_DEBUG_ALLOCATORS
            heap_allocator_tracker.before_free(ptr);
        #endif

        std::free(ptr);
    }

    void* heap_allocator_realloc(void* ptr, size_t size)
    {
        void* old_ptr = ptr;
        
        #ifdef BDC_DEBUG_ALLOCATORS 
            assert(old_ptr != nullptr);
        #endif

        char* new_ptr = reinterpret_cast<char*>(std::realloc(old_ptr, size));

        #ifdef BDC_DEBUG_ALLOCATORS                
            assert(new_ptr != nullptr);
            heap_allocator_tracker.after_realloc(old_ptr, new_ptr, size);
        #endif

        return new_ptr;
    }

    void memory_manager_init(size_t temp_buffer_size)
    {
        BDC_LOG_DEBUG("memory_manager_init() ...\n");

        BDC_LOG_DEBUG(" -- Configuring allocators ...\n");

        temp_allocator = {
            .name         = "temp_allocator",
            .proc_malloc  = &temp_allocator_malloc,
            .proc_free    = &temp_allocator_free,
            .proc_realloc = &temp_allocator_realloc
        };

        heap_allocator = {
            .name         = "heap_allocator",
            .proc_malloc  = &heap_allocator_malloc,
            .proc_free    = &heap_allocator_free,
            .proc_realloc = &heap_allocator_realloc
        };

        // Allocate temporary buffer
        BDC_LOG_DEBUG(" -- Allocating %zu bytes for the temp_allocator_buffer ...\n", temp_buffer_size);
        char* data = reinterpret_cast<char*>(std::malloc(temp_buffer_size));
        assert(data);        
        temp_allocator_buffer = {
            .data = data,
            .size = temp_buffer_size,
            .head = data
        };

        #ifdef BDC_DEBUG_ALLOCATORS
        BDC_LOG_DEBUG(" -- Setting up heap_allocator_tracker ...\n");
        heap_allocator_tracker = {
            .allocator = &heap_allocator
        };
        BDC_LOG_DEBUG(" -- Setting up temp_allocator_tracker ...\n");
        temp_allocator_tracker = {
            .allocator = &temp_allocator
        };
        #endif

        push_allocator(heap_allocator);
    }

    Memory_Manager_Report* memory_manager_generate_report(Memory_Manager_Report* report)
    {
        #ifndef BDC_DEBUG_ALLOCATORS
            assert(false && "You're trying to generate a memory report but BDC_DEBUG_ALLOCATORS must be defined in order to do this, recompile with #define BDC_DEBUG_ALLOCATORS");
        #else
        BDC_LOG_DEBUG(" -- Storing report about %s ...\n", heap_allocator_tracker.allocator->name);

        if( report == nullptr)
        {
            static Memory_Manager_Report s_report;
            report = &s_report;
        }

        *report = {};

        report->name                = heap_allocator_tracker.allocator->name;
        report->has_leaked          = heap_allocator_tracker.allocations.size() != 0;
        report->allocations.data    = heap_allocator_tracker.allocations.data();
        report->allocations.size    = heap_allocator_tracker.allocations.size();        
        #endif

        return report;
    }

    void memory_manager_shutdown()
    {
        BDC_LOG_DEBUG("memory_manager_shutdown() ...\n");

        BDC_LOG_DEBUG(" -- Empty allocator stack...\n");
        while( allocator_stack_size )
        {
            pop_allocator();
        }

        BDC_LOG_DEBUG(
            " -- Temporary allocator buffer usage is %zu Byte(s) (total available: %zu Bytes).\n"
            " -- Release temporary allocator buffer...\n",
            (size_t)temp_allocator_buffer.head - (size_t)temp_allocator_buffer.data,
            temp_allocator_buffer.size
        );
        std::free(temp_allocator_buffer.data);
        temp_allocator_buffer.data = nullptr;
    }

    void memory_manager_clear_trackers()
    {
        heap_allocator_tracker.allocations.clear();
        temp_allocator_tracker.allocations.clear();
    }

    void memory_manager_report_print(Memory_Manager_Report* report, bool asserts_no_leaks)
    {
        BDC_LOG_DEBUG("allocators_report_print() ...\n");

        if( !report->has_leaked )
        {
            BDC_LOG_DEBUG(" -- no leaks were found. Bravo \\o/\n");
            return;
        }

        BDC_LOG_DEBUG(" -- Here is the list of unfreed allocations for %s:\n", report->name);
        for(size_t i = 0; i < report->allocations.size; i++)
        {
            Memory_Allocation_Info& alloc_info = report->allocations.data[i];
            BDC_LOG_DEBUG("    -- %zu Bytes at %p\n", alloc_info.size, alloc_info.data);
        }
        
        if (asserts_no_leaks)
        {
            assert(!report->has_leaked && "Memory Leak Found!");
        }
    }

    #ifdef BDC_DEBUG_ALLOCATORS

    void Memory_Allocation_Tracker::after_malloc(void* ptr, size_t size)
    {   
        if (ptr == nullptr)
        {
            BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s was unable to malloc %lu byte(s)\n", allocator->name, size);
            return;
        }

        if( find_allocation(ptr) != nullptr )
        {
            BDC_LOG_DEBUG("[Memory_Allocation_Tracker] warning: %s already has an allocation at %p \n", allocator->name, ptr);
        }

        auto& info = allocations.emplace_back();
        info.data = ptr;
        info.size = size;

        BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s malloc %lu byte(s) at %p\n", allocator->name, size, ptr);
    }

    const Memory_Allocation_Info* Memory_Allocation_Tracker::find_allocation(void* ptr) const
    {
        assert(ptr != nullptr);
        for(auto& each : allocations)
            if (each.data == ptr)
                return &each;
        return nullptr;
    }

    void Memory_Allocation_Tracker::after_realloc(void* old_ptr, void* new_ptr, size_t new_size )
    {
        assert(old_ptr != nullptr);
        assert(new_ptr != nullptr);

        auto old = std::find_if(
            allocations.begin(), allocations.end(),
            [old_ptr](auto& item){ return item.data == old_ptr; }
        );
        assert( old != allocations.end() && "Unknown address!");

        allocations.erase(old);

        auto& info = allocations.emplace_back();
        info.data = new_ptr;
        info.size = new_size;

        BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s realloc %lu byte(s) at %p (previously: %p, %lu byte(s))\n", allocator->name, new_size, new_ptr, old->data, old->size);
    }

    void Memory_Allocation_Tracker::before_free(void* ptr)
    {               
        if( ptr == nullptr )
        {
            BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s warning: freeing nullptr will do nothing.\n", allocator->name); 
            return;
        }

        auto it = std::find_if(
            allocations.begin(), allocations.end(),
            [ptr](auto& item){ return item.data == ptr; }
        );

        if ( it == allocations.end() )
        {
            BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s warning: allocation at %p will be released but: WAS NOT REGISTERED. Will crash.\n", allocator->name, ptr); 
            return;
        }

        BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s released %p (known size: %lu)\n", allocator->name, it->data, it->size);

        allocations.erase(it);

        if ( allocations.size() == 0 )
        {
            BDC_LOG_DEBUG("[Memory_Allocation_Tracker] %s's all allocations have been released.\n", allocator->name);
        }
    }
    #endif // BDC_DEBUG_ALLOCATORS
}
