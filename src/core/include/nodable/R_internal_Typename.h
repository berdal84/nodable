#pragma once

#include "R_internal_Enum_MACROS.h"

namespace Nodable::R
{
    /**
     * To qualify a base type
     */
    enum class Qualifier : unsigned short int
    {
        Null     = 0b000,
        Ref      = 0b001,
        Pointer  = 0b010,
        Const    = 0b100,
    };

    // reflect the enum
    R_ENUM(Qualifier)
    R_ENUM_VALUE(Null)
    R_ENUM_VALUE(Ref)
    R_ENUM_VALUE(Pointer)
    R_ENUM_VALUE(Const)
    R_ENUM_END

    /**
     * To distinguish base types.
     */
    enum class Typename : unsigned short int
    {
        Null = 0,
        Void,
        Boolean,
        Double,
        String,
        Class,
        COUNT
    };

    // reflect the enum
    R_ENUM(Typename)
    R_ENUM_VALUE(Null)
    R_ENUM_VALUE(Void)
    R_ENUM_VALUE(Boolean)
    R_ENUM_VALUE(Double)
    R_ENUM_VALUE(String)
    R_ENUM_VALUE(Class)
    R_ENUM_VALUE(COUNT)
    R_ENUM_END
}
