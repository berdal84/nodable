#pragma once
#include <typeindex>
#include <unordered_set>
#include <vector>
#include <list>
#include "types.h"

namespace tools
{
    //
    // Ordered list of unique elements
    // Uses a hashset (rely on std::hash<ElementT> ) and a list to guarantee uniqueness and order.
    //
    template<typename T>
    struct UniqueOrderedList
    {
        using ElemType  = T;
        static_assert( std::is_default_constructible_v<std::hash<T>>, "std::hash<T> must be implemented");

        bool empty() const
        {
            return _unique_elem.empty();
        }

        bool contains(const ElemType& elem) const // Constant on average, worst case linear in the size of the container.
        {
            return _unique_elem.contains( _hash(elem) );
        }

        const std::list<ElemType>& data() const
        {
            return _ordered_elem;
        }

        void clear() // O(n)
        {
            if ( _unique_elem.empty() )
            {
                return;
            }
            _unique_elem.clear();
            _ordered_elem.clear();
        }

        template<class Iterator> size_t append( Iterator begin, Iterator end )
        {
            size_t count = 0;

            for(auto it = begin; it != end; ++it)
                if ( append( *it ) )
                    ++count;

            return count;
        }

        size_t size() const
        {
            return _unique_elem.size();
        }

        bool append(ElemType& elem)  // Constant on average, worst case linear in the size of the container.
        {
            const auto& [_, inserted] = _unique_elem.insert( _hash(elem) );
            if ( inserted )
            {
                _ordered_elem.push_back( elem );
                return true;
            }
            return false;
        }

        bool remove(const ElemType& elem)// Constant on average, worst case linear in the size of the container.
        {
            if ( _unique_elem.erase( _hash(elem) ) )
            {
                auto it = std::find( _ordered_elem.begin(), _ordered_elem.end(), elem );
                _ordered_elem.erase( it );
                return true;
            }
            return false;
        }

    private:
        u64_t _hash(const ElemType& elem) const
        { return std::hash<ElemType>{}(elem); }

        std::unordered_set<u64_t>          _unique_elem{};  // unordered_set, because we need uniqueness, furthermore unordered_set::contains is faster than std::find on a list
        std::list<ElemType>                _ordered_elem{}; // list, because it " supports constant time insertion and removal of elements from anywhere in the container." (see https://devdocs.io/cpp/container/list)
    };
}
