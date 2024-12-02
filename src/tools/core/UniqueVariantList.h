#pragma once
#include <typeindex>
#include <list>
#include <vector>
#include <deque>
#include <unordered_set>
#include "Signals.h"
#include "Hash.h"
#include "Variant.h"
#include "UniqueList.h"

namespace tools
{
    template<typename VariantT>
    struct UniqueVariantList;

    //
    // Extend UniqueOrderedList specifically for Variant<Args...>
    // Also provide a signal on_change to listen to append/remove events
    //
    template<typename ...Args>
    struct UniqueVariantList<Variant<Args...>>
    {
        using Element       = Variant<Args...>;
        using WrappedList   = UniqueList<Element>;
        using Iterator      = typename WrappedList::Iterator ;
        using ConstIterator = typename WrappedList::ConstIterator;

        Iterator      begin()        { return _wrapped_list.begin(); }
        Iterator      end()          { return _wrapped_list.end(); }
        ConstIterator cbegin() const { return _wrapped_list.cbegin(); }
        ConstIterator cend() const   { return _wrapped_list.cend(); }

        enum class EventType
        {
            Append,
            Remove,
        };

        SIGNAL(on_change, EventType, Element);

        Element& front()
        { return _wrapped_list.front(); }

        Element& back()
        { return _wrapped_list.back(); }

        bool empty() const
        { return _wrapped_list.empty(); }

        void clear()
        {
            for( const Element& elem : _wrapped_list.data() )
            {
                on_change.emit( EventType::Remove, elem );
            }
            _wrapped_list.clear();
            _count_by_index.clear();
        }

        bool contains(const Element& elem ) const
        { return _wrapped_list.contains(elem); }

        template<typename AlternativeT>
        bool append(AlternativeT data)
        { return append(Element{data}); } // we use add_lvalue_reference to handle pointers

        bool append(Element& elem )
        {
            const bool ok = _wrapped_list.append(elem);
            if ( ok )
            {
                _count_by_index[elem.index()]++;
                on_change.emit( EventType::Append, elem );
            }
            return ok;
        }

        template<class Iterator> size_t append( Iterator begin, Iterator end )
        {
            size_t count = 0;

            for(auto it = begin; it != end; ++it)
                if ( append( *it ) )
                    ++count;

            return count;
        }

        bool remove(const Element& elem)
        {
            const bool ok = _wrapped_list.remove(elem);
            {
                _count_by_index[elem.index()]--;
                on_change.emit( EventType::Append, elem );
            }
            return ok;
        }

        template<class AlternativeT> bool contains() const // O(1), read from cache.
        {
            constexpr size_t index = Element::template index_of<AlternativeT>();
            return _count_by_index.contains(index );
        }

        template<class AlternativeT> size_t count() const // O(1), read from cache.
        {
            constexpr size_t index = Element::template index_of<AlternativeT>();
            if ( _count_by_index.contains(index ) )
            {
                return _count_by_index.at(index );
            }
            return 0;
        }

        template<class AlternativeT>
        AlternativeT first_of() const // O(n), I suggest you to use contains() once first
        {
            const size_t _count = count<AlternativeT>();
            if ( _count == 0 )
                return {};

            for ( const Element& elem : _wrapped_list.data() )
                if ( AlternativeT ptr = elem.template get_if<AlternativeT>() )
                    return ptr;

            ASSERT(false); // unreachable case
        }

        template<class AlternativeT>
        std::vector<AlternativeT> collect() const // O(n), do only a single allocation when necessary
        {
            const size_t _count = count<AlternativeT>();
            if ( _count == 0 )
                return {};

            std::vector<AlternativeT> result;
            const std::type_index type_index{ typeid(AlternativeT) };
            result.reserve( _count ); // 1 allocation max :)

            // OPTIM: we could use a cache per type_index if necessary ( type_index => list<T*> )
            for ( const Element& elem : _wrapped_list.data() )
                if ( AlternativeT data = elem.template get_if<AlternativeT>() )
                    result.push_back( data );

            return result;
        }

    private:
        WrappedList                         _wrapped_list{};
        std::unordered_map<size_t, size_t>  _count_by_index{};
    };
}