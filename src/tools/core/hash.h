#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace tools
{
    namespace hash
    {
        inline static u32_t hash(const void* buf, size_t len, u32_t seed = 0)
        { return XXHash32::hash( buf, (uint64_t)len, seed); }

        inline static u32_t hash_cstr(const char* str, u32_t seed = 0)
        { return XXHash32::hash(str, (uint64_t)strlen(str), seed); }
    };
}
