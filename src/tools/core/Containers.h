#pragma once
#include "Asserts.h"

namespace tools
{
    template<typename ElementT>
    struct Array_View;

    template<typename ElementT>
    struct Array_Iterator;

    //
    // Inline_Vector
    // 
    // minimalist replacement for std::vector, but without allocations.
    // memory is already reserved.
    //
    template<typename ElementT, u16_t CAPACITY>
    struct Inline_Vector
    {
        using Iterator       = Array_Iterator<ElementT>;
        using Iterator_Const = Array_Iterator<const ElementT>;

        u16_t    size = {};
        ElementT data[CAPACITY] = {};

        bool empty() const
        { return size == 0; }
        
        constexpr u32_t capacity() const
        { return CAPACITY; }

        void push_back(ElementT elem)
        {
            ASSERT(size < CAPACITY);
            data[size] = elem;
            ++size;
        }

        const ElementT& operator[](u16_t index) const
        {
            ASSERT(index < size);
            return data[index];
        }

        const ElementT& at(u16_t index) const
        {
            ASSERT(index < size);
            return data[index];
        }

        ElementT& operator[](u16_t index)
        {
            ASSERT(index < size);
            return data[index];
        }

        ElementT& at(u16_t index)
        {
            ASSERT(index < size);
            return data[index];
        }

        void resize(u16_t new_size)
        {
            ASSERT(new_size <= CAPACITY);
            ASSERT(new_size > size);
            size = new_size;
        }

        Iterator erase(Iterator it)
        {
            auto pos = it - begin();

            // Move remaining elements to the left
            for(size_t i = pos; i < size - 1; i++)
            {
                data[i] = data[i+1];
            }

            --size;

            return Iterator{data + pos};
        }

        Iterator begin()
        { return Iterator{data}; }

        Iterator end()
        { return Iterator{data + size}; }

        Iterator_Const begin() const
        { return Iterator_Const{data}; }

        Iterator_Const end() const
        { return Iterator_Const{data + size}; }
    };

    // shorthands

    template<typename ElementT> using Inline_Vector8   = Inline_Vector<ElementT,   8>;
    template<typename ElementT> using Inline_Vector16  = Inline_Vector<ElementT,  16>;
    template<typename ElementT> using Inline_Vector32  = Inline_Vector<ElementT,  32>;
    template<typename ElementT> using Inline_Vector64  = Inline_Vector<ElementT,  64>;
    template<typename ElementT> using Inline_Vector128 = Inline_Vector<ElementT, 128>;
    
    template<typename ElementT>
    struct Array_View
    {
        using Iterator = Array_Iterator<ElementT>;

        ElementT* data;
        u16_t     size;

        template<typename VectorT>
        Array_View(const VectorT& vec)
        : size(vec.size)
        , data(const_cast<decltype(data)>(vec.data) )
        {}
        
        ElementT& operator[](u16_t index) const
        {
            ASSERT(index < size);
            return *(data + index);
        }

        Iterator begin() { return Iterator{data}; }
        Iterator end()   { return Iterator{data + size}; }
    };

    template<typename ElementT>
    struct Array_Iterator
    {
        Array_Iterator(ElementT* ptr):_m_ptr(ptr){}

        ElementT&       operator*() const { return *_m_ptr; }
        Array_Iterator&  operator++() { ++_m_ptr; return *this; }
        Array_Iterator&  operator++(int) { ++_m_ptr; return *this; }
        Array_Iterator&  operator+(u16_t offset) { _m_ptr += offset; return *this; }
        Array_Iterator&  operator-(u16_t offset) { _m_ptr -= offset; return *this; }
        bool            operator==(const Array_Iterator& other) const { return _m_ptr == other._m_ptr; }
        bool            operator!=(const Array_Iterator& other) const { return !(*this == other); }
        ptrdiff_t       operator-(const Array_Iterator& other) const { return _m_ptr - other._m_ptr; }

        // std compatibility
        using value_type        = ElementT;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = ptrdiff_t;
        using pointer           = ElementT*;
        using reference         = ElementT&; 

    private:
        ElementT* _m_ptr;
    };
}