#pragma once

#include <string>
#include <fw/core/types.h>

namespace fw
{
    class string // Static library to deal with string formatting
    {
    public:
        static std::string fmt_double(double);           // Format a double to a string (without trailing zeros).
        static std::string fmt_hex(u64_t _addr);         // Format a quad-word as a hexadecimal string.
        static std::string fmt_ptr(const void* _addr);   // Format an address as a hexadecimal string.
        static std::string fmt_title(const char* _title, int _width = 80); // Format a title for console output (ex: ------<=[ My Title ]=>--------)
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}