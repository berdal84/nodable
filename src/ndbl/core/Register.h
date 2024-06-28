#pragma once

#include "tools/core/types.h"
#include "tools/core/reflection/enum.h"

namespace ndbl
{
    /**
     * Enum to identify each register, we try here to follow the x86_64 DASM reference from
     * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
     */
    typedef u8_t Register;
    enum Register_ : u8_t
    {
        Register_rax = 0x00,        // primary accumulator
        Register_rdx = 0x01,        // storage
        Register_eip = 0x02,        // The instruction pointer.
        Register_COUNT,
        Register_undefined = 0xff , // undefined is the default value
    };

    REFLECT_ENUM(Register)
    REFLECT_ENUM_VALUE(Register_undefined)
    REFLECT_ENUM_VALUE(Register_rax)
    REFLECT_ENUM_VALUE(Register_rdx)
    REFLECT_ENUM_VALUE(Register_eip)
    REFLECT_ENUM_END
}
