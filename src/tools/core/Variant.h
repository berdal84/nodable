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
    // from https://stackoverflow.com/questions/52303316/get-index-by-type-in-stdVariantT
    template<typename T, typename Ts, size_t INDEX = 0>
    static constexpr size_t index_of()
    {
        constexpr size_t SIZE = std::tuple_size_v<Ts>;
        static_assert( INDEX < SIZE, "Type not found in VariantT");

        if constexpr ( std::is_same_v< T, std::tuple_element_t<INDEX, Ts >>)
            return INDEX;
        else
            return index_of<T, Ts, INDEX + 1>();
    }

    // note: we use std::monostate as first alternative type, when index() == 0, we are in that default state.
    template<typename ...Ts>
    struct VariantT
    {
        friend struct std::hash<VariantT>;

        static constexpr size_t index_null = 0; // mono state's

        VariantT() = default;
        VariantT(const VariantT&) = default;
        VariantT(VariantT&&) = default;

        template<class T>
        VariantT(T data)
        : _data( data )
        {}

        template<typename T>
        VariantT& operator=(T data)
        {
            this->_data = data;
            return *this;
        }

        VariantT& operator=(const VariantT& other) = default;
        VariantT& operator=(VariantT&& other) = default;

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
        { return VariantT::index_of<T>() == index(); }

        size_t index() const
        { return _data.index(); }

        bool operator==(const VariantT& other) const
        { return _data == other._data;  }

        bool operator!=(const VariantT& other) const
        { return !(*this == other); }

        bool empty() const
        { return _data.index() == index_null; }

        template<typename T>
        static constexpr size_t index_of()
        { return tools::index_of<T, std::tuple<std::monostate, Ts...> >(); }
    private:
        std::variant<std::monostate, Ts...> _data;
    };
}

template<typename ...Args>
struct std::hash<tools::VariantT<Args...>>
{
    u64_t operator()(const tools::VariantT<Args...>& v) const
    {
        // hash the wrapped VariantT
        return std::hash<decltype(v._data)>{}(v._data);
    }
};