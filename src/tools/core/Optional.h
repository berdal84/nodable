#pragma once

#include "assertions.h"

namespace tools
{
    //
    // Struct to deal with optional returned values
    // Implementation is very simple, it tries to follow std::optional<T> interface,
    // We only support pointers, and consider nullptr as an absence of value
    //
    template< typename TPtr, typename T = std::remove_pointer_t<TPtr> >
    struct Optional
    {

        static_assert( std::is_pointer_v<TPtr>, "Implemented for pointers only" );

        inline constexpr          Optional(): _value(nullptr) {}
        inline constexpr          Optional(nullptr_t): _value(nullptr){}
        inline constexpr          Optional(TPtr ptr): _value(ptr){}
        inline constexpr          Optional(const Optional<TPtr>& other): _value(other._value){}
        inline constexpr TPtr     operator->() { return value(); }
        inline constexpr T&       operator*() { return *value(); }
        inline constexpr          operator bool () { return has_value(); }
        inline constexpr TPtr     value_or_null() { return has_value() ? _value : nullptr; }
        inline constexpr TPtr     raw_ptr() { return _value; }
        inline constexpr TPtr     value() { VERIFY(has_value(), "no value hold"); return _value; }
        inline constexpr bool     has_value() const { return _value != nullptr; }
        inline constexpr bool     is_empty() const { return !has_value(); }
    private:
        TPtr _value;
    };

    struct S { };
    static_assert(sizeof(Optional<S*>) == 8, "Result<T*> size is incorrect" );
}
