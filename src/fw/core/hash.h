#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace fw
{
    namespace hash
    {
        inline static size_t hash(char* buffer, size_t buf_size, size_t seed = 0)
        { return XXHash32::hash(buffer, buf_size, seed); }

        inline static size_t hash(const char* str, size_t seed = 0)
        { return XXHash32::hash(str, strlen(str), seed); }
    };
}