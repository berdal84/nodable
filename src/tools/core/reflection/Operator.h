#pragma once

#include "bdc/String.hpp"
#include "Enum.h"

namespace tools
{
    enum Operator_Type: int   // To distinguish operator types
    {
        Unary   = 1,             // Unary  (ex: "-2", "++i" )
        Binary  = 2,             // Binary (ex: "1+1", "2*4", "1/2")
        Ternary = 3,             // Ternary (ex: "<condition> ? <true> : <false> )
    };

    REFLECT_ENUM_CLASS(Operator_Type)
    (
        REFLECT_ENUM_CLASS_V(Unary)
        REFLECT_ENUM_CLASS_V(Binary)
        REFLECT_ENUM_CLASS_V(Ternary)
    )

    struct Operator
    {
        bdc::String   identifier;
        Operator_Type type;
        int           precedence;
    };

    inline bool operator==(const Operator& a, const Operator& b)
    {
        return a.type == b.type && a.identifier == b.identifier; // precedence is not considered as part of the Operator's identity
    }

}// namespace tools