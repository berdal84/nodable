#pragma once

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

    /**
     * Simple structure to define an operator
     */
    class Operator
    {
    public:

        Operator() = delete;
        Operator(const Operator&) = delete;
        Operator(const std::string& _identifier, Operator_t _type, int _precedence)
                : identifier(_identifier)
                , type(_type)
                , precedence(_precedence)
        {};
        const std::string identifier;
        const int         precedence;
        const Operator_t  type;
    };
}