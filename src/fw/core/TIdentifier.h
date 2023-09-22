#pragma once
#include "types.h"
#include <type_traits>

namespace fw
{
    /**
     * Templated Identifier
     * @tparam Type to specify a type for static type-checking. void can be used to express "any type".
     * @tparam IdentifierType internal identifier type.
     */
    template<typename Type, typename IdentifierType>
    class TIdentifier
    {
        static_assert(sizeof(IdentifierType) <= sizeof(u64_t), "IdentifierType should not be greater than 64bits for performance reasons");
    public:
        using value_t = IdentifierType;
        using type = Type;
        using identity_t = TIdentifier<Type, IdentifierType>;

        static identity_t null;
        value_t m_value; // object unique identifier in a non specified context (user decides, can be an array, a pool, etc.)

        explicit constexpr TIdentifier( value_t _id = 0)
        : m_value(_id)
        {};

        constexpr TIdentifier(const identity_t& other)
        : m_value(other.m_value)
        {}

        template<typename OtherT>
        explicit TIdentifier(const TIdentifier<OtherT, value_t>& other)
        : m_value(other.m_value)
        {
            static_assert( std::is_same_v<Type, void> ||
                           std::is_same_v<Type, OtherT> || std::is_base_of_v<Type, OtherT> || std::is_base_of_v<OtherT, Type>,
                          "Type and OtherT are unrelated");
        }

        value_t id() const
        { return m_value; }

        void reset( value_t _id = null.m_value)
        { m_value = _id; }

        operator value_t() const
        { return this->m_value; }

        template<typename OtherType>
        operator TIdentifier<OtherType, value_t> () const
        { return TIdentifier<OtherType, value_t>{this->m_value}; }

        explicit operator bool () const
        { return *this != null; }

        bool operator<(const TIdentifier<Type, value_t>& other ) const
        { return m_value < other.m_value; }

        TIdentifier<Type, value_t>& operator=(const TIdentifier<Type, value_t>& other )
        { m_value = other.m_value; return *this; }

        template<typename OtherType>
        TIdentifier<Type, value_t>& operator=(const TIdentifier<OtherType, value_t>& other )
        {
            static_assert( are_related<Type, OtherType>() );
            m_value = other.m_value;
            return *this;
        }

        template<typename OtherType>
        bool operator!=(const TIdentifier<OtherType, value_t>& other ) const
        { return !are_related<Type, OtherType>() || this->m_value != other.m_value; }

        template<typename OtherType>
        bool operator==(const TIdentifier<OtherType, value_t>& other ) const
        { return are_related<Type, OtherType>() && this->m_value == other.m_value; }

        bool operator== ( value_t _id )const
        { return this->m_value == _id; }

        bool operator!= ( value_t _id )const
        { return this->m_value != _id; }

        identity_t& operator++(int increment)
        { this->m_value += increment; return *this; }

        template<typename T, typename U>
        static constexpr bool are_related()
        {
            // TODO: Too permissive?
            return std::is_same_v<T, void> || std::is_same_v<void, T> || std::is_base_of_v<T, U> || std::is_base_of_v<U, T>;
        }
    };

    template<typename T, typename U>
    TIdentifier<T, U> TIdentifier<T, U>::null{};

    // Shorthand to IDs with 8, 16, 32, or 64 bits.
    template<typename T> using ID8  = TIdentifier<T, u8_t>;
    template<typename T> using ID16 = TIdentifier<T, u16_t>;
    template<typename T> using ID32 = TIdentifier<T, u32_t>;
    template<typename T> using ID64 = TIdentifier<T, u64_t>;

    class C {};
    static_assert( sizeof(ID8<C>)  == 1, "should match");
    static_assert( sizeof(ID16<C>) == 2, "should match");
    static_assert( sizeof(ID32<C>) == 4, "should match");
    static_assert( sizeof(ID64<C>) == 8, "should match");

    // Shorthand to 32-bit id
    template<typename T> using ID = TIdentifier<T, u32_t>;
}