#pragma once

#include "R_MACROS.h"

namespace Nodable::R
{
    /**
     * To distinguish base types.
     */
    enum class Type : unsigned short int
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
    R_ENUM(Type)
    R_ENUM_VALUE(Null)
    R_ENUM_VALUE(Void)
    R_ENUM_VALUE(Boolean)
    R_ENUM_VALUE(Double)
    R_ENUM_VALUE(String)
    R_ENUM_VALUE(Class)
    R_ENUM_VALUE(COUNT)
    R_ENUM_END
}
