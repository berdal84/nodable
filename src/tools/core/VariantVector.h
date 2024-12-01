#pragma once
#include <typeindex>
#include <list>
#include <vector>
#include <deque>
#include <unordered_set>
#include <variant>
#include <iterator> // for forward_iterator, see append
#include "Signals.h"
#include "Hash.h"

namespace tools
{
    // from https://stackoverflow.com/questions/52303316/get-index-by-type-in-stdvariant
    template<typename T, typename Ts, size_t INDEX = 0>
    static constexpr size_t index_of()
    {
        constexpr size_t SIZE = std::tuple_size_v<Ts>;
        static_assert( INDEX < SIZE, "Type not found in variant");

        if constexpr ( std::is_same_v< T, std::tuple_element_t<INDEX, Ts >>)
            return INDEX;
        else
            return index_of<T, Ts, INDEX + 1>();
    }

    // note: we use std::monostate as first alternative type, when index() == 0, we are in that default state.
    template<typename ...Ts>
    struct Variant
    {
        static constexpr size_t index_null = 0; // mono state's

        Variant() = default;

        template<class T>
        Variant(T data)
        : _data( data )
        {}

        template<typename T>
        Variant& operator=(T data)
        {
            this->_data = data;
            return *this;
        }

        Variant& operator=(const Variant& other) = default;

        template<class T>
        T get() const
        { return std::get<T>(_data); }

        template<class T>
        T get_if() const
        {
            if ( holds_alternative<T>() )
                return std::get<T>(_data);
            return {};
        }

        template<typename T>
        constexpr bool holds_alternative() const
        { return Variant::index_of<T>() == index(); }

        size_t index() const
        { return _data.index(); }

        bool  operator==(void* ptr) const // check whether *ptr is held by this AnyPointer, useful for std::find
        {  return _data == ptr; };

        bool operator==(const Variant& other) const
        { return Hash::hash32(_data) == Hash::hash32(other._data );  }

        bool operator!=(const Variant& other) const
        { return !(*this == other); }

        bool empty() const
        { return _data.index() == index_null; }

        template<typename T>
        static constexpr size_t index_of()
        { return tools::index_of<T, std::tuple<std::monostate, Ts...> >(); }

    private:
        std::variant<std::monostate, Ts...> _data;
    };

    template<typename ...Args>
    struct VariantVector
    {
        using element_t = Variant<Args...>;

        enum EventT
        {
            EventT_Append,
            EventT_Remove,
        };

        SIGNAL(on_change, EventT, element_t);

        bool empty() const
        {
            return _unique_hash.empty();
        }

        bool contains(const element_t& data) const // Constant on average, worst case linear in the size of the container.
        {
            return _unique_hash.contains( Hash::hash32(data));
        }

        const std::list<element_t>& data() const
        {
            return _ordered_elem;
        }

        void clear() // O(n)
        {
            for( const element_t& elem : _ordered_elem )
            {
                on_change.emit( EventT_Remove, elem );
            }
            _unique_hash.clear();
            _ordered_elem.clear();
            _count_by_index.clear();
        }

        template<class Iterator> size_t append( Iterator begin, Iterator end )
        {
            size_t count = 0;

            for(auto it = begin; it != end; ++it)
                if ( append( element_t{*it} ) )
                    ++count;

            return count;
        }

        template<class T> bool contains() const // O(1), read from cache.
        {
            constexpr size_t index = element_t::template index_of<T>();
            return _count_by_index.contains( index );
        }

        template<class T> size_t count() const // O(1), read from cache.
        {
            constexpr size_t index = element_t::template index_of<T>();
            if ( _count_by_index.contains( index ) )
            {
                return _count_by_index.at( index );
            }
            return 0;
        }

        bool append(element_t elem)  // Constant on average, worst case linear in the size of the container.
        {
            const auto& [_, inserted] = _unique_hash.insert( Hash::hash32(elem) );
            if ( inserted )
            {
                _ordered_elem.push_back(elem);
                on_change.emit( EventT_Append, elem );
                _count_by_index[elem.index()]++;
                return true;
            }
            return false;
        }

        bool remove(const element_t& data)// Constant on average, worst case linear in the size of the container.
        {
            if ( _unique_hash.erase( Hash::hash32(data) ) )
            {
                auto it = std::find( _ordered_elem.begin(), _ordered_elem.end(), data );
                on_change.emit( EventT_Remove, data );
                _ordered_elem.erase( it );
                _count_by_index[it->index()]--;
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

            for ( const element_t& elem : _ordered_elem )
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
            for ( const element_t& elem : _ordered_elem )
                if ( T data = elem.template get_if<T>() )
                    result.push_back( data );

            return result;
        }
    private:
        std::unordered_set<u32_t >         _unique_hash{};  // unordered_set because we need uniqueness, furthermore unordered_set::contains is faster than std::find on a list
        std::list<element_t>               _ordered_elem{}; // list, because it " supports constant time insertion and removal of elements from anywhere in the container." (see https://devdocs.io/cpp/container/list)
        std::unordered_map<size_t, size_t> _count_by_index{};
    };
}