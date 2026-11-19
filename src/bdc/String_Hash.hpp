#pragma once

#include "String.hpp"
#include "Types.hpp"
#include "Type_Traits.hpp"
#include "Hash.hpp"

namespace bdc
{
    struct String_Hash
    {
        u32_t   hash;    // hashed this->string
        String  string;  // view to the original string (Remi did this, let's try and see, could be useful to debug)
    };

    constexpr inline String_Hash string_hash(const String& str)
    {
        const u32_t hash = djb2_hash(str.data, str.size);
        return { .hash = hash ? hash : 0, .string = str };
    }

    inline bool string_hash_is_null(const String_Hash& sh)
    {
        return sh.hash == 0;
    }

    inline bool operator==(const String_Hash& a, const String_Hash& b)
    {
        return a.hash == b.hash;
    }

    inline bool operator!=(const String_Hash& a, const String_Hash& b)
    {
        return a.hash != b.hash;
    }
}
