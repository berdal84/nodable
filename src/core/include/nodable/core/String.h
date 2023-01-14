#pragma once

#include <string>
#include <nodable/core/types.h>

namespace ndbl
{
    /**
     * @class Static library to deal with string formatting
     */
    class String
    {
    public:
        /** Format a double to a string (without trailing zeros)*/
        static std::string fmt_double(double);
        /** Format an qword as an hexadecimal string */
        static std::string fmt_hex(u64_t _addr);
        /** Format an address as an hexadecimal string */
        static std::string fmt_ptr(const void* _addr);
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}