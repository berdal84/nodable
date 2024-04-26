#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace fw
{
    namespace hash
    {
        using hash_t = u32_t;

        inline static hash_t hash(char* buffer, size_t buf_size, uint32_t seed = 0)
        { return XXHash32::hash(buffer, (uint64_t)buf_size, seed); }

        inline static hash_t hash(const char* str, uint32_t seed = 0)
        { return XXHash32::hash(str, (uint64_t)strlen(str), seed); }
    };
}