#pragma once

#include "tools/core/reflection/enum.h"

namespace ndbl::assembly
{
    /**
     * Enum to identify each register, we try here to follow the x86_64 DASM reference from
     * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
     */
    enum Register : u8_t {
        rax = 0,        // primary accumulator
        rdx = 1,        // storage
        eip = 2,        // The instruction pointer.
        COUNT,
        undefined = std::numeric_limits<u8_t>::max(),  // undefined is the default value
    };

    R_ENUM(Register)
        R_ENUM_VALUE(undefined)
        R_ENUM_VALUE(rax)
        R_ENUM_VALUE(rdx)
        R_ENUM_VALUE(eip)
    R_ENUM_END
} // namespace nodable::assembly
