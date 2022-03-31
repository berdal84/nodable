#pragma once

#include <nodable/core/types.h>

namespace Nodable
{
namespace assembly
{
    /**
     * Enum to identify each register, we try here to follow the x86_64 DASM reference from
     * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
     */
    enum Register : u8_t {
        undefined,
        rax,     // primary accumulator
        rdx,     // storage
        eip,     // The instruction pointer.
        COUNT
    };

    R_ENUM(Register)
        R_ENUM_VALUE(undefined)
        R_ENUM_VALUE(rax)
        R_ENUM_VALUE(rdx)
        R_ENUM_VALUE(eip)
    R_ENUM_END
} // namespace Asm
} // namespace Nodable