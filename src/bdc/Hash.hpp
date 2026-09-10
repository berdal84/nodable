#pragma once
#include "Types.hpp"

namespace bdc
{
    inline u32_t djb2_hash(const char* data, size_t size)
    {
        u32_t hash  = 5381;
        u32_t i     = 0;
        while ( i < size )
            hash = ((hash << 5) + hash) + data[i++]; /* hash * 33 + str[i] */
        return hash;
    }   
}