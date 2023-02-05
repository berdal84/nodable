#pragma once

#include <string>
#include "types.h"

namespace fw
{
    /**
     * @class Static library to deal with string formatting
     */
    class String
    {
    public:
        static std::string fmt_double(double);           // Format a double to a string (without trailing zeros).
        static std::string fmt_hex(u64_t _addr);         // Format a quad-word as a hexadecimal string.
        static std::string fmt_ptr(const void* _addr);   // Format an address as a hexadecimal string.
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}