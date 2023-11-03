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
        Token           token_for;
        PoolID<Node>    init_instr();
        PoolID<Node>    iter_instr();
        void            init() override;
    private:
        ID8<Slot>       m_init_slot;
        ID8<Slot>       m_iter_slot;

        REFLECT_DERIVED_CLASS()
    };
}
