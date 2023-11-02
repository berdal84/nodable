#pragma once

#include "TConditionalNode.h"
#include "Token.h"
#include <memory>

namespace ndbl
{
    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "for"
     * for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode : public TConditionalNode<2>
    {
    public:
        Token                   token_for;
        PoolID<InstructionNode> init_instr;
        PoolID<InstructionNode> iter_instr;
        void                    init() override;
        REFLECT_DERIVED_CLASS()
    };
}
