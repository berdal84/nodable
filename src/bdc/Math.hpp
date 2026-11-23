#pragma once
#include "Types.hpp"

namespace bdc
{
    inline u32_t u32_round_up_to_power_of_2(u32_t n)
    {
        if(n == 0)
            return 1;
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }

} // namespace bdc