#pragma once

#include <string>
#include <nodable/core/types.h>

namespace Nodable
{
    class String
    {
    public:
        static std::string fmt_double(double);
        static std::string fmt_hex(u64_t _addr);
        static std::string fmt_ptr(const void* _addr);
    private:
        static void limit_trailing_zeros(std::string& str, int _trailing_max);
    };
}