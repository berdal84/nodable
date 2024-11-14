#pragma once

#include <memory>
#include "tools/core/reflection/reflection"

#include "SwitchBehavior.h"// base class
#include "Token.h"

namespace ndbl
{
    /**
     * @class Represent a conditional and iterative structure "while"
     * while( condition_expr ) {
     *   // do something
     * }
     */
    class WhileLoopNode : public Node, public SwitchBehavior
    {
    public:
        REFLECT_DERIVED_CLASS()

        Token token_while = { Token_t::keyword_while};

        void init(const std::string& _name);
    };
}
