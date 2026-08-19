#include "String_Hash.hpp"

namespace bdc
{
    u32_t djb2_hash(const String& str)
    {
        u32_t hash  = 5381;
        u32_t i     = 0;
        while ( i < str.size )
            hash = ((hash << 5) + hash) + str[i++]; /* hash * 33 + str[i] */
        return hash;
    }   

    String_Hash string_hash(const String& str)
    {
        return { .hash = djb2_hash(str), .string = str };
    }

    bool operator==(const String_Hash& a, const String_Hash& b)
    {
        return a.hash == b.hash;
    }

    bool operator!=(const String_Hash& a, const String_Hash& b)
    {
        return a.hash != b.hash;
    }
} // namespace bdc