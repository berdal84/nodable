#pragma once
#include <nodable/core/reflection/reflection>

namespace ndbl
{

    enum class Operator_t: int   // To distinguish operator types
    {
        Unary   = 1,             // Unary  (ex: "-2", "++i" )
        Binary  = 2,             // Binary (ex: "1+1", "2*4", "1/2")
        Ternary = 3,             // Ternary (ex: "<condition> ? <true> : <false> )
    };

    R_ENUM(Operator_t)
    R_ENUM_VALUE(Unary)
    R_ENUM_VALUE(Binary)
    R_ENUM_VALUE(Ternary)
    R_ENUM_END
}