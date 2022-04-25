#pragma once

#include <string>
#include <nodable/core/Operator_t.h>

namespace ndbl
{
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