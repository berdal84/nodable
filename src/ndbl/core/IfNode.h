#pragma once

#include "IScope.h"
#include "Node.h"
#include "TConditional.h"
#include "Token.h"
#include <memory>
#include <utility>

namespace ndbl
{

    /**
     * @class Represent a conditional structure with two branches ( IF/ELSE )
     * @note Multiple ConditionalNode can be chained to form an IF / ELSE IF / ... / ELSE.
     */
    class IfNode : public Node
    {
    public:
        Token token_if   = {Token_t::keyword_if};
        Token token_else = {Token_t::keyword_else};
        void           init(const std::string& _name);
        Scope*         scope_at(Branch branch) const       { return m_wrapped_conditional.get_scope_at(branch); }
        Slot*          child_slot_at(Branch branch)        { return m_wrapped_conditional.get_child_slot_at(branch); }
        const Slot*    child_slot_at(Branch branch) const  { return m_wrapped_conditional.get_child_slot_at(branch); }
        Slot*          condition_slot(Branch branch)       { return m_wrapped_conditional.get_condition_slot(branch); }
        const Slot*    condition_slot(Branch branch) const { return m_wrapped_conditional.get_condition_slot(branch); }
        Node*          condition(Branch branch)            { return m_wrapped_conditional.get_condition(branch); }
        const Node*    condition(Branch branch) const      { return m_wrapped_conditional.get_condition(branch); }
    private:
        TConditional<2> m_wrapped_conditional;
        REFLECT_DERIVED_CLASS()
    };
}
