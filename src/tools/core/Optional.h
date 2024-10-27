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
        template<typename TOther>
        inline constexpr          Optional(const Optional<TOther>& other): _value( other.data() ){}

        inline constexpr TPtr     data() const { return _value; } // like a no-check get()
        inline constexpr TPtr     get() const { VERIFY(valid(), "no value hold"); return _value; }
        inline constexpr bool     valid() const { return _value != nullptr; }
        inline constexpr bool     empty() const { return !valid(); }
        inline void               reset() { _value = {}; }

        inline constexpr TPtr     operator->() const { return get(); }
        inline constexpr T&       operator*() const { return *get(); }
        inline constexpr          operator bool () const { return valid(); }
        inline bool operator==(const T* other) const { return other == _value; }
        inline bool operator!=(const T* other) const { return other != _value; }
    private:
        TPtr _value;
    };

    struct S { };
    static_assert(sizeof(Optional<S*>) == 8, "Result<T*> size is incorrect" );
}
