#pragma once

#include <memory>
#include "tools/core/reflection/reflection"

#include "ASTConditionalT.h"// base class
#include "ASTToken.h"

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
    class ASTWhileLoopNode : public ASTNode
    {
    public:
        ASTToken          token_while;

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
