#pragma once

#include "TConditional.h"
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
    class ForLoopNode : public Node
    {
    public:
        Token          token_for;

        void           init(const std::string& _name);
        Slot&          initialization_slot();
        Slot&          iteration_slot();
        const Slot&    initialization_slot() const;
        const Slot&    iteration_slot() const;
        Scope*         scope_at(Branch branch) const       { return m_wrapped_conditional.get_scope_at(branch); }
        Slot&          child_slot_at(Branch branch)        { return m_wrapped_conditional.get_child_slot_at(branch); }
        const Slot&    child_slot_at(Branch branch) const  { return m_wrapped_conditional.get_child_slot_at(branch); }
        Slot&          condition_slot(Branch branch)       { return m_wrapped_conditional.get_condition_slot(branch); }
        const Slot&    condition_slot(Branch branch) const { return m_wrapped_conditional.get_condition_slot(branch); }
        Node*          condition(Branch branch) const      { return m_wrapped_conditional.get_condition(branch); }
    private:
        TConditional<2> m_wrapped_conditional;
        Slot*           m_initialization_slot{nullptr};
        Slot*           m_iteration_slot{nullptr};

        REFLECT_DERIVED_CLASS()
    };
}
