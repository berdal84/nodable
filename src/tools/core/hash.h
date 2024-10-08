#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace tools
{
    namespace hash
    {
        inline static u32_t hash(char* buffer, size_t buf_size, u32_t seed = 0)
        { return XXHash32::hash(buffer, (uint64_t)buf_size, seed); }

        inline static u32_t hash(const char* str, u32_t seed = 0)
        { return XXHash32::hash(str, (uint64_t)strlen(str), seed); }
    };
}
