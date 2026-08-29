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

        Array(Elem_Type* _data, u32_t _size): data(_data), size(_size) {}
        Array(const std::initializer_list<Elem_Type>& list)
        : data( const_cast<i8_t*>(&*list.begin()) )
        , size( list.size() )
        {}
            
        inline const Elem_Type& operator[](u32_t pos) const
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }

        inline Elem_Type& operator[](u32_t pos)
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }
    };

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
            
        Resizable_Array(Allocator* _allocator = default_allocator() )
        : data(nullptr)
        , size(0)
        , capacity(0)
        , allocator( _allocator )
        {}

        inline const Elem_Type& operator[](u32_t pos) const
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }

        inline Elem_Type& operator[](u32_t pos)
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }

        // I tried to use to_array() instead, see at the end of this file.
        // inline operator Array<Elem_Type>& () { return *reinterpret_cast<Array<Elem_Type>*>(this); }
        // inline operator const Array<Elem_Type>& () const { return *reinterpret_cast<Array<Elem_Type>*>(this); }
    };

    //
    // Inlined_Array is like a fixed-capacity Array<T> that is aware of its (fixed-)capacity.
    //
    template<typename Elem_Type, u32_t CAPACITY>
    struct Inlined_Array
    {
        u32_t   size;
        i8_t    data[CAPACITY]; // data must be 2nd to match with other arrays

        constexpr u32_t capacity() const
        { return CAPACITY; }

        inline const Elem_Type& operator[](u32_t pos) const
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }

        inline Elem_Type& operator[](u32_t pos)
        {
            assert(pos < size && "out of bounds");
            return *(((Elem_Type*)data) + pos);
        }
    };

    // Some templates to deduce if a type is an Hash_Map and get its sub types
    template<typename T>
    concept Any_Array_Type =
        (requires(T& t) {
            []<typename Elem_Type>(Array<Elem_Type>&){}(t);
        }) ||
        (requires(T& t) {
            []<typename Elem_Type>(Resizable_Array<Elem_Type>&){}(t);
        }) ||   
        (requires(T& t) {
            []<typename Elem_Type, u32_t CAPACITY>(Inlined_Array<Elem_Type, CAPACITY>&  ){}(t);
        });

    // Generic Array API (works on both Array and Resizable_Array)

    template<Any_Array_Type T>
    inline typename T::Elem_Type& array_front(T& arr)
    {
        assert(arr.size);
        return arr[0];
    }

    template<Any_Array_Type T>
    inline typename T::Elem_Type& array_back(T& arr)
    {
        assert(arr.size);
        return arr[arr.size-1];
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

    template<typename Elem_Type, u32_t CAPACITY>
    Elem_Type array_remove(Inlined_Array<Elem_Type, CAPACITY>& arr, u32_t pos)
    {
        assert( pos <= arr.size );
        Elem_Type removed_elem = arr[pos];
        for(size_t i = pos; i < arr.size; ++i )
        {
            arr[i] = arr[i+1];
        }
        arr.size -= 1;
        return removed_elem;
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
            memset( (void*)(&arr[arr.size]), 0, (new_capacity - arr.size) * sizeof(Elem_Type)); // new elements are zero-initialized
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

    template<typename Elem_Type>
    Array<Elem_Type> to_view(const Resizable_Array<Elem_Type>& arr)
    {
        Array<Elem_Type> view{};
        view.data = arr.data;
        view.size = arr.size;
        
        return view;
    }
} // namespace bdc
