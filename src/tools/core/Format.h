#pragma once

#include <cassert>
#include <cstring>
#include <chrono>
#include <xxhash/xxhash64.h>

#include "bdc/String.hpp"
#include "bdc/Types.hpp"

namespace tools
{
    namespace Format
    {
        bdc::String number(double);             // Format a double to a string (without trailing zeros).
        bdc::String hexadecimal(u64_t n);       // Format a quad-word as a hexadecimal string.
        bdc::String address(const void* addr);  // Format an address as a hexadecimal string.
        bdc::String title(const bdc::String& /* title */ , u32_t width = 80); // Format a title for console output (ex: ------<=[ My Title ]=>--------)
        bdc::String time_point_to_string(const std::chrono::system_clock::time_point&);
        void limit_trailing_zeros(bdc::String& str, int _trailing_max);

    }
}