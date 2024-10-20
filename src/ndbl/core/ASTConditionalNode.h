#pragma once

#include "ASTScopeInterface.h"
#include "ASTNode.h"
#include "ASTConditionalT.h"
#include "ASTToken.h"
#include <memory>
#include <utility>

namespace ndbl
{

    /**
     * @class Represent a conditional structure with two branches ( IF/ELSE )
     * @note Multiple ConditionalNode can be chained to form an IF / ELSE IF / ... / ELSE.
     */
    class ASTConditionalNode : public ASTNode
    {
    public:
        ASTToken token_if;   // Example: { prefix: "", word: "if", suffix: " "}
        ASTToken token_else; // Example: { prefix: " ", word: "else", suffix: " "}
        void           init(const std::string& _name);
        ASTScope*      scope_at(Branch branch) const       { return m_wrapped_conditional.get_scope_at(branch); }
        Slot&          child_slot_at(Branch branch)        { return m_wrapped_conditional.get_child_slot_at(branch); }
        const Slot&    child_slot_at(Branch branch) const  { return m_wrapped_conditional.get_child_slot_at(branch); }
        Slot&          condition_slot(Branch branch)       { return m_wrapped_conditional.get_condition_slot(branch); }
        const Slot&    condition_slot(Branch branch) const { return m_wrapped_conditional.get_condition_slot(branch); }
        ASTNode*       condition(Branch branch) const      { return m_wrapped_conditional.get_condition(branch); }
    private:
        ASTConditionalT<2> m_wrapped_conditional;
        REFLECT_DERIVED_CLASS()
    };
}
