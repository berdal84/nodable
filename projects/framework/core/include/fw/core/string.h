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
        template<size_t width = 80>
        static std::string fmt_title(const char* _title) // Format a title for console output (ex: ------<=[ My Title ]=>--------)
        {
            /*
             * Takes _title and do:
             * ------------<=[ _title ]=>------------
             */

            const char* pre       = "-=[ ";
            const char* post      = " ]=-";
            const char* padding   = "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
                                    "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-";
            int pad_size = (width - strlen(_title) - strlen(pre) - strlen(post)) / 2;

            char result[width+1]; // _width + end of line
            snprintf(result, width, "%*.*s%s%s%s%*.*s\n",
                     0, pad_size, padding,
                     pre, _title, post,
                     0, pad_size-1, padding
            );
            result[width] = '\0';
            return result;
        }
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