#pragma once
#include <array>
#include "assertions.h"

namespace tools
{
    template<typename ElementT>
    struct ArrayView;

    template<typename ElementT>
    struct ArrayIterator;

    //
    // InlineVector
    // 
    // minimalist replacement for std::vector, but without allocations.
    // memory is already reserved.
    //
    template<typename ElementT, u16_t CAPACITY>
    struct InlineVector
    {
        using Iterator      = ArrayIterator<ElementT>;
        using IteratorConst = ArrayIterator<const ElementT>;

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

            std::move(it+1, end(), it);

            --size;

            return Iterator{data + pos};
        }

        Iterator begin()
        { return Iterator{data}; }

        Iterator end()
        { return Iterator{data + size}; }

        IteratorConst begin() const
        { return IteratorConst{data}; }

        IteratorConst end() const
        { return IteratorConst{data + size}; }
    };

    // shorthands

    template<typename ElementT> using InlineVector8   = InlineVector<ElementT,   8>;
    template<typename ElementT> using InlineVector16  = InlineVector<ElementT,  16>;
    template<typename ElementT> using InlineVector32  = InlineVector<ElementT,  32>;
    template<typename ElementT> using InlineVector64  = InlineVector<ElementT,  64>;
    template<typename ElementT> using InlineVector128 = InlineVector<ElementT, 128>;
    
    template<typename ElementT>
    struct ArrayView
    {
        using Iterator = ArrayIterator<ElementT>;

        ElementT* data;
        u16_t     size;

        template<typename VectorT>
        ArrayView(const VectorT& vec)
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
    struct ArrayIterator
    {
        ArrayIterator(ElementT* ptr):_m_ptr(ptr){}

        ElementT&       operator*() const { return *_m_ptr; }
        ArrayIterator&  operator++() { ++_m_ptr; return *this; }
        ArrayIterator&  operator++(int) { ++_m_ptr; return *this; }
        ArrayIterator&  operator+(u16_t offset) { _m_ptr += offset; return *this; }
        ArrayIterator&  operator-(u16_t offset) { _m_ptr -= offset; return *this; }
        bool            operator==(const ArrayIterator& other) const { return _m_ptr == other._m_ptr; }
        bool            operator!=(const ArrayIterator& other) const { return !(*this == other); }
        ptrdiff_t       operator-(const ArrayIterator& other) const { return _m_ptr - other._m_ptr; }

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