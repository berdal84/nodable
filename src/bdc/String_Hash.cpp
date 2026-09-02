#include "String_Hash.hpp"

namespace bdc
{
    inline u32_t djb2_hash(const String& str)
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
} // namespace bdc