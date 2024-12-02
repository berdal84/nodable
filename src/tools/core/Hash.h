#pragma once

#include <cstring>
#include <xxhash/xxhash32.h>

namespace tools
{
    struct Hash
    {
        constexpr static u64_t DEFAULT_UNSIZED = -1;
        constexpr static u64_t DEFAULT_SEED    = 20241984;

        template<typename T>
        static u64_t hash(const T& data, u64_t size = DEFAULT_UNSIZED )
        {
            if constexpr ( !std::is_pointer_v<T> ) // objects
            {
                if( size == DEFAULT_UNSIZED )
                    size = 1;
                return _hash(&data, sizeof(T) * size);
            }
            else if constexpr ( std::is_same_v<const char*, T> )  // cstring
            {
                if( size == DEFAULT_UNSIZED )
                    size = strlen(data);
                return _hash(data, size );
            }
            else
            {
                return _hash(data, sizeof(void*)); // raw pointers
            }
        }

        static u64_t _hash(const void* str, u64_t size, u64_t seed = DEFAULT_SEED)
        {
            return XXHash64::hash(str, size, seed);
        }
    };
}
