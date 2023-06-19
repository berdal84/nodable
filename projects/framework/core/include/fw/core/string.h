#pragma once

#include <string>
#include <cstring>
#include <fw/core/types.h>
#include <xxhash/xxhash64.h>

namespace fw
{
    class string // Static library to deal with string formatting
    {
    public:
        static std::string fmt_double(double);           // Format a double to a string (without trailing zeros).
        static std::string fmt_hex(u64_t _addr);         // Format a quad-word as a hexadecimal string.
        static std::string fmt_ptr(const void* _addr);   // Format an address as a hexadecimal string.
        static std::string fmt_title(const char* _title, int _width = 80); // Format a title for console output (ex: ------<=[ My Title ]=>--------)
        inline static size_t hash(char* buffer, size_t buf_size, size_t seed = 0)
        {
            return XXHash64::hash(buffer, buf_size, seed);
        }
        inline static size_t hash(const char* str, size_t seed = 0)
        {
            return XXHash64::hash(str, strlen(str), seed);
        }
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}