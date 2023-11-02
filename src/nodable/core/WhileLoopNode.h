#pragma once

#include <memory>
#include "fw/core/reflection/reflection"

#include "TConditionalNode.h"// base class
#include "Token.h"

namespace ndbl
{
    // forward declarations
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "while"
     * while( condition_expr ) {
     *   // do something
     * }
     */
    class WhileLoopNode : public TConditionalNode<2> {
    public:
        Token token_while;
        WhileLoopNode();
        REFLECT_DERIVED_CLASS()
    };
}
