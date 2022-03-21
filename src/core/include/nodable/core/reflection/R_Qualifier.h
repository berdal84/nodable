#pragma once

#include "R_MACROS.h"

namespace Nodable { namespace R
{
    /**
     * To qualify a base type
     */
    enum class Qualifier : unsigned short int
    {
        None     = 0,
        Ref      = 1 << 1,
        Pointer  = 1 << 2,
        Const    = 1 << 3,
    };

    // reflect the enum
    R_ENUM(Qualifier)
    R_ENUM_VALUE(None)
    R_ENUM_VALUE(Ref)
    R_ENUM_VALUE(Pointer)
    R_ENUM_VALUE(Const)
    R_ENUM_END
} }
