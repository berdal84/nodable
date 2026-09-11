#pragma once

#include <cassert>
#include <cstring>
#include <initializer_list> // for std::initializer_list

#include "Allocators.hpp"
#include "Types.hpp"
#include "Type_Traits.hpp"
#include "Math.hpp"

namespace bdc
{
    template<typename _Elem_T>
    struct Array
    {
        using Elem_T = _Elem_T;

        u32_t   size; // size must be 1st to be the same type as Inlined_Array
        Elem_T* data;

        Array()             = default;
        Array(const Array&) = default;
        ~Array()            = default;

        Array(u32_t _size, Elem_T* _data): data(_data), size(_size) {}
        Array(const std::initializer_list<Elem_T>& list)
        : data(const_cast<Elem_T*>(&*list.begin()) )
        , size( list.size() )
        {}

        inline const Elem_T& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T*       begin()        { return data; }
        inline Elem_T*       end()          { return data + size; }
        inline const Elem_T* begin() const { return data; }
        inline const Elem_T* end() const   { return data + size; }
    };

    // Simply declare array_join, user must implement it. Only array_join for String is implement in String.hpp.
    template<typename Elem_T>
    Elem_T array_join(const Array<Elem_T>& array, const Elem_T& separator );

    
    //
    // Resizable_Array<T> is like a Array<T> memory wise, but is aware of its buffer capacity and allocator.
    //
    template<typename _Elem_T>
    struct Resizable_Array
    {
        using Elem_T = _Elem_T;

        u32_t       size = 0; // size must be 1st to be the same type as Inlined_Array
        Elem_T*     data = 0;
        u32_t       capacity = 0;
        Allocator*  allocator = 0;
            
        Resizable_Array() = default;
        Resizable_Array(const Resizable_Array& ) = default;
        ~Resizable_Array() = default;

        inline const Elem_T& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T*       begin()        { return data; }
        inline Elem_T*       end()          { return data + size; }
        inline const Elem_T* begin() const { return data; }
        inline const Elem_T* end() const   { return data + size; }
    };

    //
    // Inlined_Array is like a fixed-capacity Array<T> that is aware of its (fixed-)capacity.
    //
    template<typename _Elem_T, u32_t CAPACITY>
    struct Inlined_Array
    {
        using Elem_T = _Elem_T;

        u32_t     size = 0;
        Elem_T    data[CAPACITY]; // data must be 2nd to match with other arrays

        constexpr u32_t capacity() const
        { return CAPACITY; }

        inline const Elem_T& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return data[pos]; }
        inline Elem_T*       begin()        { return data; }
        inline Elem_T*       end()          { return data + size; }
        inline const Elem_T* begin() const { return data; }
        inline const Elem_T* end() const   { return data + size; }
    };
    static_assert( std::is_default_constructible_v<Inlined_Array<i8_t, 16>> );
    
    // Minimal concept to know if a given type is compatible with the generic array API
    template<typename T>
    concept Is_Array = requires(T& t, u32_t pos)
    {
        { t.size } -> std::convertible_to<u32_t>;
        t.data;
    };
    
    // Generic Array API (works on Array, Inline_Array and Resizable_Array)

    template<Is_Array Array_Type, typename Elem_T = Array_Type::Elem_T>
    inline Array<Elem_T> array_view(const Array_Type& arr)
    {
        return Array<Elem_T>( arr.size, (Elem_T*)arr.data );
    }

    template<Is_Array T>
    inline T::Elem_T& array_front(T& arr)
    {
        assert(arr.size);
        return arr[0];
    }

    template<Is_Array T>
    inline T::Elem_T& array_back(T& arr)
    {
        assert(arr.size);
        return arr[arr.size-1];
    }

    template<Is_Array T>
    inline const T::Elem_T& array_back(const T& arr)
    {
        assert(arr.size);
        return arr[arr.size-1];
    }

    struct Array_Find_Result
    {
        bool  found  = false;
        u32_t at_pos = (u32_t)-1;
    };

    template<Is_Array T>
    Array_Find_Result array_find(T& arr, const typename T::Elem_T& elem)
    {
        for(u32_t i = 0; i < arr.size; i++)
            if ( arr[i] == elem )
                return { .found = true, .at_pos = i };

        return { .found = false };
    }

    template<Is_Array T>
    Array_Find_Result array_rfind(T& arr, const typename T::Elem_T& elem)
    {
        if( arr.size == 0)
            return { .found = false };

        for(u32_t i = arr.size-1; i >= 0; i--)
            if ( arr[i] == elem )
                return { .found = true, .at_pos = i };

        return { .found = false };
    }
    
    template<Is_Array T>
    Ref_Or_Ptr<typename T::Elem_T> array_append(T& arr, Ref_Or_Ptr<const typename T::Elem_T> elem)
    {
        array_resize(arr, arr.size + 1 );
        arr[arr.size-1] = elem;
        return arr[arr.size-1];
    }

    template<Is_Array T>
    void array_remove_ordered(T& arr, u32_t pos)
    {
        assert( pos <= arr.size );

        for(u32_t i = pos; i < arr.size-1; ++i )
            arr[i] = arr[i+1];

        arr.size -= 1;
    }

    template<Is_Array T>
    void array_remove_unordered(T& arr, u32_t pos)
    {
        static_assert(false, "array_remove_unordered is not implemented yet");
    }

    // Inlined_Array specific API

    template<typename Elem_T, u32_t CAPACITY>
    void array_init(Inlined_Array<Elem_T, CAPACITY>& arr, u32_t initial_size = 0)
    {
        if ( initial_size )
            array_resize(arr, initial_size);
    }

    template<typename Elem_T, u32_t CAPACITY>
    void array_resize(Inlined_Array<Elem_T, CAPACITY>& arr, u32_t new_size)
    {
        assert( new_size <= arr.capacity() );
        arr.size = new_size;
    }

    // Resizable_Array specific API

    template<typename Elem_T>
    void array_init(Resizable_Array<Elem_T>& arr, u32_t initial_capacity = 0, Allocator* _allocator = nullptr)
    {
        arr.size        = 0;
        arr.data        = nullptr;
        arr.capacity    = 0;
        arr.allocator   = _allocator ? _allocator : allocator;
        array_ensure_has_capacity(arr, initial_capacity);
    }

    template<typename Elem_T>
    void array_release(Resizable_Array<Elem_T>& arr)
    {
        if(arr.data == nullptr)
        {
            return;
        }
        
        memory_free(arr.data, arr.allocator);

        arr.data     = nullptr;
        arr.size     = 0;
        arr.capacity = 0;
    }

    inline u32_t array_compute_capacity_from_size(u32_t new_size, u32_t capacity_min)
    {
        const u32_t capacity = u32_round_up_to_power_of_2(new_size);
        if( capacity < capacity_min )
        {
            return capacity_min;
        }
        return capacity;
    }

    template<typename Elem_T>
    void array_resize(Resizable_Array<Elem_T>& arr, u32_t new_size)
    {
        if( arr.capacity < new_size )
        {
            u32_t capacity = array_compute_capacity_from_size(new_size, 16);
            array_ensure_has_capacity(arr, capacity);
        }

        for( u32_t i = arr.size; i < new_size; ++i)
        {
            new (arr.data + i) Elem_T();
        }
        arr.size = new_size;
    }

    template<typename Elem_T>
    void array_ensure_has_capacity(Resizable_Array<Elem_T>& arr, u32_t capacity)
    {
        if( arr.capacity >= capacity )
        {
            return;
        }

        assert(arr.allocator != nullptr && "arr.allocator is required to reserve memory");

        if( arr.data == nullptr )
        {
            arr.data = memory_malloc_array<Elem_T>(capacity, arr.allocator);
        }
        else
        {
            arr.data = memory_realloc_array<Elem_T>(arr.data, capacity, arr.allocator);
        }
        assert(arr.data != nullptr);
        memset( (void*)(arr.data + arr.capacity), 0, (capacity - arr.capacity) * sizeof(Elem_T)); // new elements are zero-initialized
        arr.capacity = capacity;
    }

    template<typename Elem_T>
    void array_append(Resizable_Array<Elem_T>& arr, std::initializer_list<Elem_T>&& list)
    {
        for( auto& each : list)
            array_append(arr, each);
    }

    template<typename Elem_T>
    Array<Elem_T> array_concat(const Array<Elem_T>& a, const Array<Elem_T>& b, Allocator* allocator )
    {
        Resizable_Array<Elem_T> result{};
        array_resize(result, a.size + b.size);
        memcpy( result.data   , a.data, a.size);
        memcpy(&result[a.size], b.data, b.size);

        return result;
    }

} // namespace bdc
