#pragma once
#include <typeindex>
#include <list>
#include <vector>
#include <deque>
#include <unordered_set>
#include "Signals.h"
#include "Hash.h"

#if TOOLS_DEBUG
#include "assertions.h"
#define UNIQUE_ORDERED_LIST_ASSERT(...) ASSERT( __VA_ARGS__ )
#else
#define UNIQUE_ORDERED_LIST_ASSERT(...)
#endif

namespace tools
{
    template<typename ElementT>
    struct UniqueOrderedVariantList
    {
        UniqueOrderedVariantList() = delete;
        ~UniqueOrderedVariantList() = delete;
    };

    //
    // Ordered list of unique elements
    // Uses a hashset (rely on std::hash<ElementT> ) and a list to guarantee uniqueness and order.
    //
    template<typename ...Args>
    struct UniqueOrderedVariantList<Variant<Args...>>
    {
        using ElementT = Variant<Args...>;
        static_assert( std::is_default_constructible_v<std::hash<ElementT>>, "std::hash<ElementT> must be implemented");

        enum class EventType
        {
            Append,
            Remove,
        };

        SIGNAL(on_change, EventType, ElementT);

        bool empty() const
        {
            return _unique_elem.empty();
        }

        bool contains(const ElementT& elem) const // Constant on average, worst case linear in the size of the container.
        {
            return _unique_elem.contains( _hash(elem) );
        }

        const std::list<ElementT>& data() const
        {
            return _ordered_elem;
        }

        void clear() // O(n)
        {
            if ( _unique_elem.empty() )
            {
                return;
            }

            for( const ElementT& elem : _ordered_elem )
            {
                on_change.emit( EventType::Remove, elem );
            }
            _unique_elem.clear();
            _ordered_elem.clear();
            _count_by_index.clear();
        }

        template<class Iterator> size_t append( Iterator begin, Iterator end )
        {
            size_t count = 0;

            for(auto it = begin; it != end; ++it)
                if ( append( *it ) )
                    ++count;

            return count;
        }

        template<class OtherT> bool contains() const // O(1), read from cache.
        {
            constexpr size_t index = ElementT::template index_of<OtherT>();
            return _count_by_index.contains( index );
        }

        template<class T> size_t count() const // O(1), read from cache.
        {
            constexpr size_t index = ElementT::template index_of<T>();
            if ( _count_by_index.contains( index ) )
            {
                return _count_by_index.at( index );
            }
            return 0;
        }

        template<typename T>
        bool append(T* ptr)
        {
            ElementT elem{ptr};
            return append( elem );
        }

        template<typename T>
        bool append(T& data)
        {
            return append(ElementT{data});
        }

        bool append(ElementT& elem)  // Constant on average, worst case linear in the size of the container.
        {
            const auto& [_, inserted] = _unique_elem.insert( _hash(elem) );
            if ( inserted )
            {
                _ordered_elem.push_back( elem );
                on_change.emit( EventType::Append, elem );
                _count_by_index[elem.index()]++;
                UNIQUE_ORDERED_LIST_ASSERT( contains( elem ) );
                return true;
            }
            return false;
        }

        bool remove(const ElementT& elem)// Constant on average, worst case linear in the size of the container.
        {
            if ( _unique_elem.erase( _hash(elem) ) )
            {
                auto it = std::find( _ordered_elem.begin(), _ordered_elem.end(), elem );
                on_change.emit( EventType::Remove, elem );
                _ordered_elem.erase( it );
                _count_by_index[it->index()]--;
                UNIQUE_ORDERED_LIST_ASSERT( !contains( elem ) );
                return true;
            }
            return false;
        }

        template<class T>
        T first_of() const // O(n), I suggest you to use contains() once first
        {
            const size_t _count = count<T>();
            if ( _count == 0 )
                return {};

            for ( const ElementT& elem : _ordered_elem )
                if ( T ptr = elem.template get_if<T>() )
                    return ptr;

            return nullptr;
        }

        template<class T>
        std::vector<T> collect() const // O(n), do only a single allocation when necessary
        {
            const size_t _count = count<T>();
            if ( _count == 0 )
                return {};

            std::vector<T> result;
            const std::type_index type_index{ typeid(T) };
            result.reserve( _count ); // 1 allocation max :)

            // OPTIM: we could use a cache per type_index if necessary ( type_index => list<T*> )
            for ( const ElementT& elem : _ordered_elem )
                if ( T data = elem.template get_if<T>() )
                    result.push_back( data );

            return result;
        }

        u64_t _hash(const ElementT& elem) const
        { return std::hash<ElementT>{}(elem); }

    private:
        std::unordered_set<u64_t>          _unique_elem{};  // unordered_set, because we need uniqueness, furthermore unordered_set::contains is faster than std::find on a list
        std::list<ElementT>                _ordered_elem{}; // list, because it " supports constant time insertion and removal of elements from anywhere in the container." (see https://devdocs.io/cpp/container/list)
        std::unordered_map<size_t, size_t> _count_by_index{};
    };
}