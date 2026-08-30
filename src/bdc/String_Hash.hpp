#pragma once
#include "String.hpp"
#include "Types.hpp"

namespace bdc
{
    struct String_Hash
    {
        u32_t   hash;    // hashed this->string
        String  string;  // view to the original string (Remi did this, let's try and see, could be useful to debug)
    };
    static_assert( std::is_trivially_constructible_v<String_Hash> );

    String_Hash string_hash(const String& str);


    inline bool operator==(const String_Hash& a, const String_Hash& b)
    {
        return a.hash == b.hash;
    }

    inline bool operator!=(const String_Hash& a, const String_Hash& b)
    {
        return a.hash != b.hash;
    }
}
