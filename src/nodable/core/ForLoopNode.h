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
     * for( <init>; <condition>; <iteration> ) { ... }
     */
    class ForLoopNode : public TConditionalNode<2>
    {
    public:
        Token           token_for;
        void            init() override;
        Slot&           initialization_slot();
        Slot&           iteration_slot();
    private:
        ID8<Slot>       m_initialization_slot;
        ID8<Slot>       m_iteration_slot;

        REFLECT_DERIVED_CLASS()
    };
}
