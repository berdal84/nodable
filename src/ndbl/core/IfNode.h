#pragma once

#include "IConditional.h"
#include "IScope.h"
#include "Node.h"
#include "TConditionalNode.h"
#include "Token.h"
#include <memory>
#include <utility>

namespace ndbl
{

    /**
     * @class Represent a conditional structure with two branches ( IF/ELSE )
     * @note Multiple ConditionalNode can be chained to form an IF / ELSE IF / ... / ELSE.
     */
    class IfNode : public TConditionalNode<2>
    {
    public:
        Token token_if;   // Example: { prefix: "", word: "if", suffix: " "}
        Token token_else; // Example: { prefix: " ", word: "else", suffix: " "}
        IfNode();
        REFLECT_DERIVED_CLASS()
    };
}
