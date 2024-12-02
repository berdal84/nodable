#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace tools
{
    struct Hash
    {
        constexpr static u32_t DEFAULT_UNSIZED = -1;
        constexpr static u32_t DEFAULT_SEED    = 20241984;

        template<typename T>
        static u32_t hash32(const T& data, u32_t size = DEFAULT_UNSIZED )
        {
            if constexpr ( !std::is_pointer_v<T> ) // objects
            {
                if( size == DEFAULT_UNSIZED )
                    size = 1;
                return _hash32(&data, sizeof(T) * size);
            }
            else if constexpr ( std::is_same_v<const char*, T> )  // cstring
            {
                if( size == DEFAULT_UNSIZED )
                    size = strlen(data);
                return _hash32(data, size );
            }
            else
            {
                return _hash32(data, sizeof(void*)); // raw pointers
            }
        }

        static u32_t _hash32(const void* str, u32_t size, u32_t seed = DEFAULT_SEED)
        {
            return XXHash32::hash(str, size, seed);
        }
    };
}
