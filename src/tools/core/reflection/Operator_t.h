#pragma once
#include "enum.h"

namespace tools
{

    enum class Operator_t: int   // To distinguish operator types
    {
        Unary   = 1,             // Unary  (ex: "-2", "++i" )
        Binary  = 2,             // Binary (ex: "1+1", "2*4", "1/2")
        Ternary = 3,             // Ternary (ex: "<condition> ? <true> : <false> )
    };

    REFLECT_ENUM_CLASS(Operator_t)
    (
        REFLECT_ENUM_CLASS_V(Unary)
        REFLECT_ENUM_CLASS_V(Binary)
        REFLECT_ENUM_CLASS_V(Ternary)
    )
}