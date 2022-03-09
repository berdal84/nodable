#pragma once

#include "R_MACROS.h"

namespace Nodable::R
{
    /**
     * To qualify a base type
     */
    enum class TypeQualifier : unsigned short int
    {
        None     = 0,
        Ref      = 1 << 1,
        Pointer  = 1 << 2,
        Const    = 1 << 3,
    };

    // reflect the enum
    R_ENUM(TypeQualifier)
    R_ENUM_VALUE(None)
    R_ENUM_VALUE(Ref)
    R_ENUM_VALUE(Pointer)
    R_ENUM_VALUE(Const)
    R_ENUM_END
}
