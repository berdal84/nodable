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
     * for( <init_ex>; <condition>; <iteration> ) { ... }
     */
    class ForLoopNode : public TConditionalNode<2>
    {
    public:
        Token           token_for;
        void            init(const std::string& _name);
        Slot&           initialization_slot();
        Slot&           iteration_slot();
        const Slot&     initialization_slot() const;
        const Slot&     iteration_slot() const;
    private:
        Slot*           m_initialization_slot{nullptr};
        Slot*           m_iteration_slot{nullptr};
        REFLECT_DERIVED_CLASS()
    };
}
