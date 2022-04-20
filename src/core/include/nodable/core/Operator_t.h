#pragma once
#include <nodable/core/reflection/reflection>

namespace Nodable
{

    enum class Operator_t: int
    {
        Unary   = 1,
        Binary  = 2,
        Ternary = 3,
    };

    R_ENUM(Operator_t)
    R_ENUM_VALUE(Unary)
    R_ENUM_VALUE(Binary)
    R_ENUM_VALUE(Ternary)
    R_ENUM_END
}