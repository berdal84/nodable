#pragma once

#include "./string.h"
#include "./types.h"
#include <cassert>
#include <cstring>
#include <string>
#include <chrono>
#include <xxhash/xxhash64.h>

namespace fw
{
    namespace format
    {
        std::string number(double);               // Format a double to a string (without trailing zeros).
        std::string hexadecimal(u64_t _addr);     // Format a quad-word as a hexadecimal string.
        std::string address(const void* _addr);   // Format an address as a hexadecimal string.
        template<size_t width = 80>
        std::string title(const char* _title) // Format a title for console output (ex: ------<=[ My Title ]=>--------)
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
        void limit_trailing_zeros(std::string& str, int _trailing_max);
        fw::string32 time_point_to_string(const std::chrono::system_clock::time_point&);
    };
}