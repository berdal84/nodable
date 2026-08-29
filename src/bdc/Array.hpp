#pragma once

#include "Allocators.hpp"
#include <cassert>
#include <cstring>
#include <initializer_list>

namespace bdc
{
    typedef unsigned int u32_t;

    template<typename _Elem_Type>
    struct Array
    {
        using Elem_Type = _Elem_Type;

        static constexpr u32_t npos = (u32_t)-1; // invalid position, depends on context

        u32_t size; // size must be 1st to be the same type as Inlined_Array
        i8_t* data;

        Array()             = default;
        Array(const Array&) = default;
        ~Array()            = default;

        Array(u32_t _size, i8_t* _data): data((i8_t*)_data), size(_size) {}
        Array(const std::initializer_list<Elem_Type>& list)
        : data( const_cast<i8_t*>(&*list.begin()) )
        , size( list.size() )
        {}

        inline const Elem_Type& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type*       begin()        { return (Elem_Type*)data; }
        inline Elem_Type*       end()          { return ((Elem_Type*)data) + size; }
        inline const Elem_Type* begin() const { return (Elem_Type*)data; }
        inline const Elem_Type* end() const   { return ((Elem_Type*)data) + size; }
    };
    static_assert( std::is_trivially_constructible_v<Array<i8_t>> );


    // Simply declare array_join, user must implement it. Only array_join for String is implement in String.hpp.
    template<typename Elem_Type>
    Elem_Type array_join(const Array<Elem_Type>& array, const Elem_Type& separator, Allocator* string_allocator = temp_allocator() );

    //
    // Resizable_Array<T> is like a Array<T> memory wise, but is aware of its buffer capacity and allocator.
    //
    template<typename _Elem_Type>
    struct Resizable_Array
    {
        using Elem_Type = _Elem_Type;

        u32_t       size; // size must be 1st to be the same type as Inlined_Array
        i8_t*       data;
        u32_t       capacity;
        Allocator*  allocator;
            
        Resizable_Array() = default;
        Resizable_Array(const Resizable_Array& ) = default;
        ~Resizable_Array() = default;

        inline const Elem_Type& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type*       begin()        { return (Elem_Type*)data; }
        inline Elem_Type*       end()          { return ((Elem_Type*)data) + size; }
        inline const Elem_Type* begin() const { return (Elem_Type*)data; }
        inline const Elem_Type* end() const   { return ((Elem_Type*)data) + size; }
    };
    static_assert( std::is_trivially_constructible_v<Resizable_Array<i8_t>> );

    //
    // Inlined_Array is like a fixed-capacity Array<T> that is aware of its (fixed-)capacity.
    //
    template<typename _Elem_Type, u32_t CAPACITY>
    struct Inlined_Array
    {
        using Elem_Type = _Elem_Type;

        u32_t   size;
        i8_t    data[CAPACITY]; // data must be 2nd to match with other arrays

        constexpr u32_t capacity() const
        { return CAPACITY; }

        inline const Elem_Type& operator[](u32_t pos) const { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type&       operator[](u32_t pos)       { assert(pos < size && "out of bounds");return *(((Elem_Type*)data) + pos); }
        inline Elem_Type*       begin()        { return (Elem_Type*)data; }
        inline Elem_Type*       end()          { return ((Elem_Type*)data) + size; }
        inline const Elem_Type* begin() const { return (Elem_Type*)data; }
        inline const Elem_Type* end() const   { return ((Elem_Type*)data) + size; }
    };
    static_assert( std::is_trivially_constructible_v<Inlined_Array<i8_t, 16>> );
    
    // Concept to know if a give type is compatible with the generic array API
    template<typename T>
    concept Is_Array = requires(T& t, u32_t pos)
    {
        { t.size } -> std::convertible_to<u32_t>;
        t.data;
    };
    
    // Generic Array API (works on both Array and Resizable_Array)

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    inline Array<Elem_Type> array_view(const Array_Type& arr)
    {
        return Array<Elem_Type>( arr.size, (i8_t*)arr.data );
    }

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    inline Elem_Type& array_front(Array_Type& arr)
    {
        assert(arr.size);
        return arr[0];
    }

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    inline Elem_Type& array_back(Array_Type& arr)
    {
        assert(arr.size);
        return arr[arr.size-1];
    }

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    inline const Elem_Type& array_back(const Array_Type& arr)
    {
        assert(arr.size);
        return arr[arr.size-1];
    }

    struct Array_Find_Result
    {
        bool  found  = false;
        u32_t at_pos = (u32_t)-1;
    };

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    Array_Find_Result array_find(Array_Type& arr, const Elem_Type& elem)
    {
        for(u32_t i = 0; i < arr.size; i++)
            if ( arr[i] == elem )
                return { .found = true, .at_pos = i };

        return { .found = false };
    }

    template<Is_Array Array_Type, typename Elem_Type = Array_Type::Elem_Type>
    Array_Find_Result array_rfind(Array_Type& arr, const Elem_Type& elem)
    {
        if( arr.size == 0)
            return { .found = false };

        for(u32_t i = arr.size-1; i >= 0; i--)
            if ( arr[i] == elem )
                return { .found = true, .at_pos = i };

        return { .found = false };
    }

    template<Is_Array Array_Type>
    void array_remove_ordered(Array_Type& arr, u32_t pos)
    {
        assert( pos <= arr.size );

        for(u32_t i = pos; i < arr.size-1; ++i )
            arr[i] = arr[i+1];

        arr.size -= 1;
    }

    // Inlined_Array API

    template<typename Elem_Type, u32_t CAPACITY>
    void array_init(Inlined_Array<Elem_Type, CAPACITY>& arr, u32_t initial_size = 0)
    {
        array_resize(arr, initial_size);
        memset(arr.data, 0, sizeof(arr.data) );
    }

    template<typename Elem_Type, u32_t CAPACITY>
    void array_resize(Inlined_Array<Elem_Type, CAPACITY>& arr, u32_t new_size)
    {
        assert( new_size <= arr.capacity() );
        arr.size = new_size;
    }

    template<typename Elem_Type, u32_t CAPACITY>
    Elem_Type& array_append(Inlined_Array<Elem_Type, CAPACITY>& arr, const Elem_Type& elem)
    {
        assert(arr.size < CAPACITY);
        return arr[arr.size++] = elem;
    }

    // Resizable_Array API

    template<typename Elem_Type>
    void array_init(Resizable_Array<Elem_Type>& arr, u32_t initial_capacity, Allocator* allocator = default_allocator() )
    {
        arr.size        = 0;
        arr.data        = nullptr;
        arr.capacity    = 0;
        arr.allocator   = allocator;
        array_ensure_has_capacity(arr, initial_capacity);
    }

    template<typename Elem_Type>
    void array_release(Resizable_Array<Elem_Type>& arr)
    {
        assert(arr.allocator != nullptr);
        
        memory_free(arr.data, arr.allocator);

        arr.data     = nullptr;
        arr.size     = 0;
        arr.capacity = 0;
    }

    template<typename Elem_Type>
    void array_resize(Resizable_Array<Elem_Type>& arr, u32_t new_size)
    {
        array_ensure_has_capacity(arr, new_size);
        arr.size = new_size;
    }

    template<typename Elem_Type>
    void array_ensure_has_capacity(Resizable_Array<Elem_Type>& arr, u32_t new_capacity)
    {
        //
        // TODO: exponential capacity grow.
        //       currently an allocation is done each time we resize!!!
        //

        assert(new_capacity >= arr.capacity);
        assert(arr.allocator != nullptr);

        // needs to allocate/reallocate?
        if( new_capacity > arr.capacity )
        {
            if( arr.data == nullptr )
            {
                arr.data = memory_malloc_array<i8_t>(new_capacity * sizeof(Elem_Type), arr.allocator);
            }
            else
            {
                arr.data = memory_realloc_array(arr.data, new_capacity * sizeof(Elem_Type), arr.allocator);
            }
            assert(arr.data != nullptr);
            memset( (void*)(arr.data + arr.size * sizeof(Elem_Type) ), 0, (new_capacity - arr.size) * sizeof(Elem_Type)); // new elements are zero-initialized
            arr.capacity = new_capacity;
        }        
    }

    template<typename Elem_Type>
    void array_append(Resizable_Array<Elem_Type>& arr, const Elem_Type& elem)
    {
        u32_t index = arr.size;
        array_resize( arr, arr.size + 1 );
        arr[index] = elem;
    }

    template<typename Elem_Type>
    Array<Elem_Type> array_concat(const Array<Elem_Type>& a, const Array<Elem_Type>& b, Allocator* allocator )
    {
        Resizable_Array<Elem_Type> result{};
        array_resize(result, a.size + b.size);
        memcpy( result.data   , a.data, a.size * sizeof(Elem_Type));
        memcpy(&result[a.size], b.data, b.size * sizeof(Elem_Type));

        return result;
    }

} // namespace bdc
