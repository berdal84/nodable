#pragma once

#include <memory>
#include "tools/core/reflection/reflection"

#include "TConditional.h"// base class
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
    class WhileLoopNode : public Node
    {
    public:
        Token token_while = { Token_t::keyword_while};

        void               init(const std::string& _name);
        inline Scope*      scope_at(Branch branch) const       { return m_wrapped_conditional.get_scope_at(branch); }
        inline Slot*       child_slot_at(Branch branch)        { return m_wrapped_conditional.get_child_slot_at(branch); }
        inline const Slot* child_slot_at(Branch branch) const  { return m_wrapped_conditional.get_child_slot_at(branch); }
        inline Slot*       condition_slot(Branch branch)       { return m_wrapped_conditional.get_condition_slot(branch); }
        inline const Slot* condition_slot(Branch branch) const { return m_wrapped_conditional.get_condition_slot(branch); }
        inline Node*       condition(Branch branch)            { return m_wrapped_conditional.get_condition(branch); }
        inline const Node* condition(Branch branch) const      { return m_wrapped_conditional.get_condition(branch); }
    private:
        TConditional<2> m_wrapped_conditional;
        REFLECT_DERIVED_CLASS()
    };
}
