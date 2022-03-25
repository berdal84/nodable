#pragma once

#include <string>
#include <nodable/core/types.h>

namespace Nodable
{
    class Format
    {
    public:
        static std::string fmt_no_trail(double);
        static std::string fmt_hex(u64 _addr);
        static std::string fmt_ptr(const void* _addr);
    private:
        static void remove_trailing_zeros(std::string& str);
    };
}