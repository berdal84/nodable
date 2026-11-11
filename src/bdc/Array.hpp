#pragma once

#include <cstring>
#include <initializer_list>
#include <algorithm> // for std::copy
#include "MACROS.hpp"
#include "Allocators.hpp"
#include "Types.hpp"
#include "Type_Traits.hpp"
#include "Math.hpp"

namespace bdc
{   
    template<typename _Elem_T>
    struct Resizable_Array;

    template<typename _Elem_T, u32_t CAPACITY>
    struct Inlined_Array;

    template<typename _Elem_T>
    struct Array
    {
        using Elem_T = _Elem_T;

        u32_t   size; // In order to match memory layout with Inlined_Array<N>, we declare size first and data second.
        Elem_T* data;

        constexpr Array()             = default;
        constexpr Array(const Array&) = default;
        constexpr inline Array(u32_t /* size */, Elem_T* /* data */);
        template<size_t N>
        constexpr inline Array(Elem_T (&/* array_literal */)[N] );

        // Array(const Resizable_Array<Elem_T>& arr)
        // : size(arr.size)
        // , data(const_cast<Elem_T*>(arr.data) )
        // {}

        // template<u32_t CAPACITY>
        // Array(const Inlined_Array<Elem_T, CAPACITY>& arr)
        // : size(CAPACITY)
        // , data(const_cast<Elem_T*>(arr.data) )
        // {}

        ~Array() = default;

        inline const Elem_T& operator[](u32_t pos) const    { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)          { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T*       begin()                        { return data; }
        inline Elem_T*       end()                          { return data + size; }
        inline const Elem_T* begin() const                  { return data; }
        inline const Elem_T* end() const                    { return data + size; }
    };

    template<typename Elem_T>
    constexpr inline Array<Elem_T>::Array(u32_t _size, Elem_T* _data)
    : data(_data)
    , size(_size)
    {}

    template<typename Elem_T> // yeah, I have to do this in CPP to declare my implementation outside my struct...
    template<size_t N>        // ... two template<...>. Crazy sh!t.
    constexpr inline Array<Elem_T>::Array(Elem_T (&array_literal)[N])
    : size(N)
    , data(reinterpret_cast<Elem_T*>(&array_literal))
    {
        static_assert( N < (u32_t)-1, "array size must fit in u32_t");
    }

    //
    // Resizable_Array<T> is like a Array<T> memory wise, but is aware of its buffer capacity and allocator.
    //
    template<typename _Elem_T>
    struct Resizable_Array
    {
        using Elem_T = _Elem_T;

        u32_t       size        = 0; // In order to match memory layout with Inlined_Array<N>, we declare size first and data second.
        Elem_T*     data        = 0;

        u32_t       capacity    = 0; // Can't put this close to 'size', because size and data must be together to match Inlined_Array<N> layout.
        Allocator*  allocator;
            
        Resizable_Array(): allocator(bdc::allocator) {}
        Resizable_Array(Allocator* _allocator): allocator(_allocator) {}
        Resizable_Array(const Resizable_Array& ) = default;
        ~Resizable_Array() = default;

        inline operator Array<Elem_T>() const               { return { size, data }; }
        inline const Elem_T& operator[](u32_t pos) const    { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)          { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T*       begin()                        { return data; }
        inline Elem_T*       end()                          { return data + size; }
        inline const Elem_T* begin() const                  { return data; }
        inline const Elem_T* end() const                    { return data + size; }
    };

    template<typename T>
    Resizable_Array<T> resizable_array(u32_t size, Allocator* _allocator = allocator)
    {
        Resizable_Array<T> result{ _allocator };
        array_resize(result, size);
        return result;
    }

    //
    // Inlined_Array is like a fixed-capacity Array<T> that is aware of its (fixed-)capacity.
    //
    template<typename _Elem_T, u32_t CAPACITY>
    struct Inlined_Array
    {
        using Elem_T = _Elem_T;

        u32_t                   size            = 0;
        Elem_T                  data[CAPACITY]  = {};       // since capacity is unknown, we must put it second. This way all array types have some common memory layout for its members (size, data).
        static constexpr u32_t  capacity        = CAPACITY;
        
        constexpr inline Inlined_Array() = default;
        constexpr inline Inlined_Array(const std::initializer_list<Elem_T>& list);
        template<size_t N>
        constexpr inline Inlined_Array(Elem_T (&/* array_literal */)[N] );

        inline operator Array<Elem_T>() const               { return { size, (Elem_T*)data }; }
        inline const Elem_T& operator[](u32_t pos) const    { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T&       operator[](u32_t pos)          { BDC_ASSERT(pos < size, "out of bounds"); return data[pos]; }
        inline Elem_T*       begin()                        { return data; }
        inline Elem_T*       end()                          { return data + size; }
        inline const Elem_T* begin() const                  { return data; }
        inline const Elem_T* end() const                    { return data + size; }
    };
    static_assert( std::is_default_constructible_v<Inlined_Array<i8_t, 16>> );
    
    template<typename Elem_T, u32_t CAPACITY>
    constexpr inline Inlined_Array<Elem_T, CAPACITY>::Inlined_Array(const std::initializer_list<Elem_T>& list)
    : size(list.size())
    , data()
    {
        BDC_ASSERT(list.size() <= CAPACITY, "Statically allocated array size must fit CAPACITY");
        std::copy( list.begin(), list.end(), data );
    }

    template<typename Elem_T, u32_t CAPACITY> // yeah, I have to do this in CPP to declare my implementation outside my struct...
    template<size_t N>        // ... two template<...>. Crazy sh!t.
    constexpr inline Inlined_Array<Elem_T, CAPACITY>::Inlined_Array(Elem_T (&array_literal)[N])
    : size(N)
    , data()
    {
        static_assert( N <= CAPACITY, "Statically allocated array size must fit in u32_t");
        std::copy( (&array_literal), (&array_literal) + N, &data );
    }

    // Minimal concept to know if a given type is compatible with the generic array API
    template<typename T>
    concept Is_Array = requires(T& t, u32_t pos)
    {
        { t.size } -> std::convertible_to<u32_t>;
        t.data;
    };
    
    // Generic Array API (works on Array, Inline_Array and Resizable_Array)

    // template<Is_Array Array_Type, typename Elem_T = Array_Type::Elem_T>
    // inline Array<Elem_T> array_view(const Array_Type& arr)
    // {
    //     return Array<Elem_T>( arr.size, (Elem_T*)arr.data );
    // }


    template<Is_Array T>
    inline T::Elem_T& array_front(T& arr)
    {
        BDC_ASSERT(arr.size);
        return arr[0];
    }

    template<Is_Array T>
    inline T::Elem_T& array_back(T& arr)
    {
        BDC_ASSERT(arr.size);
        return arr[arr.size-1];
    }

    template<Is_Array T>
    inline const T::Elem_T& array_back(const T& arr)
    {
        BDC_ASSERT(arr.size);
        return arr[arr.size-1];
    }

    struct Array_Find_Result
    {
        bool  ok     = false;
        u32_t at_pos = (u32_t)-1;
    };

    template<Is_Array T>
    Array_Find_Result array_find(T& arr, const typename T::Elem_T& elem)
    {
        for(u32_t i = 0; i < arr.size; i++)
            if ( arr[i] == elem )
                return { .ok = true, .at_pos = i };

        return { .ok = false };
    }

    template<Is_Array T>
    Array_Find_Result array_rfind(T& arr, const typename T::Elem_T& elem)
    {
        if( arr.size == 0)
            return { .ok = false };

        for(u32_t i = arr.size-1; i >= 0; i--)
            if ( arr[i] == elem )
                return { .ok = true, .at_pos = i };

        return { .ok = false };
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
        array_remove_range_ordered(arr, pos, 1);
    }

    template<Is_Array T>
    void array_pop(T& arr)
    {
        BDC_ASSERT( arr.size != 0 );
        array_resize(arr, arr.size-1);
    }

    template<Is_Array T>
    void array_remove_unordered(T& arr, u32_t pos)
    {
        BDC_ASSERT( pos < arr.size );

        if( pos != arr.size -1)
        {
            std::swap(arr[pos], array_back(arr));
        }        
        
        array_resize(arr, arr.size-1);
    }

    template<Is_Array T>
    void array_remove_range_ordered(T& arr, u32_t pos, u32_t count)
    {
        BDC_ASSERT( pos + count <= arr.size );

        // Move the trailing items to the left
        u32_t i = pos;
        while( i + count < arr.size )
        {
            arr[i] = arr[i + count];
            ++i;
        }

        array_resize(arr, arr.size-count);
    }

    template<Is_Array T>
    void array_clear(T& arr)
    {
        array_resize(arr, 0);
    }

    template<typename T>
    T array_copy(const T& arr, Allocator* _allocator = bdc::allocator)
    {
        T result = resizable_array<typename T::Elem_T>(arr.size, _allocator);
        std::copy(arr.begin(), arr.end(), result.data);
        return result;
    }

    template<Is_Array T>
    T array_tcopy(const T& arr)
    {
        return array_copy(arr, &temp_allocator);
    }

    // Inlined_Array specific API

    template<typename Elem_T, u32_t CAPACITY>
    void array_resize(Inlined_Array<Elem_T, CAPACITY>& arr, u32_t new_size)
    {
        BDC_ASSERT( new_size <= arr.capacity );
        if( new_size == arr.size )
        {
            return;
        }

        // on grow, call constructor for the n new element(s)
        if( new_size > arr.size )
        {            
            for( u32_t n = arr.size; n < new_size; ++n )
            {
                new (arr.data + n) Elem_T();
            }
        }
        // on shrink, call destructor for the n extra element(s)
        else
        {
            
            for( u32_t n = new_size; n < arr.size; ++n)
            {
                (arr.data + n)->~Elem_T();
            }
        }

        arr.size = new_size;
    }

    // Resizable_Array specific API

    template<typename Elem_T>
    void array_release(Resizable_Array<Elem_T>& arr)
    {
        if(arr.data == nullptr)
        {
            return;
        }
        
        for( u32_t n = 0; n < arr.size; ++n)
        {
            (arr.data + n)->~Elem_T();
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
        if( arr.size == new_size )
        {
            return;
        }

        if( arr.capacity < new_size )
        {
            u32_t capacity = array_compute_capacity_from_size(new_size, 16);
            array_ensure_has_capacity(arr, capacity);
        }

        // on grow, call constructor for the n new element(s)
        if( new_size > arr.size )
        {            
            for( u32_t n = arr.size; n < new_size; ++n )
            {
                new (arr.data + n) Elem_T();
            }
        }
        // on shrink, call destructor for the n extra element(s)
        else
        {
            
            for( u32_t n = new_size; n < arr.size; ++n)
            {
                (arr.data + n)->~Elem_T();
            }
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

        BDC_ASSERT(arr.allocator != nullptr, "arr.allocator is required to reserve memory");

        if( arr.data == nullptr )
        {
            arr.data = memory_malloc_array<Elem_T>(capacity, arr.allocator);
        }
        else
        {
            arr.data = memory_realloc_array<Elem_T>(arr.data, capacity, arr.allocator);
        }
        BDC_ASSERT(arr.data != nullptr);
        memset( (void*)(arr.data + arr.capacity), 0, (capacity - arr.capacity) * sizeof(Elem_T)); // new elements are zero-initialized
        arr.capacity = capacity;
    }

    // Note: consider using array_concat (faster) instead .
    template<typename Elem_T>
    void array_append(Resizable_Array<Elem_T>& arr, const std::initializer_list<Elem_T>& other)
    {
        for(auto& each : other)
        {
            array_append(arr, each);
        }
    }

    template<Is_Array A, Is_Array B, typename Elem_T = A::Elem_T>
    Array<Elem_T> array_tconcat(const A& a, const B& b)
    {
        Resizable_Array<Elem_T> result{};
        result.allocator = &temp_allocator;
        return array_concat(result, a, b);
    }

    template<Is_Array A, Is_Array B, typename Elem_T = A::Elem_T>
    Resizable_Array<Elem_T>& array_concat(Resizable_Array<Elem_T>& out, const A& a, const B& b)
    {
        u32_t out_initial_size = out.size;
        array_resize(out, out.size + a.size + b.size);
        
        // Note: constructor needs to be called, we do not distinguish POD from other data structures yet.

        std::copy(a.begin(), a.end(), out.begin() + out_initial_size);
        std::copy(b.begin(), b.end(), out.begin() + out_initial_size + a.size);

        return out;
    }


} // namespace bdc
