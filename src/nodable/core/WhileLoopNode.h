#pragma once

#include <memory>
#include "fw/core/reflection/reflection"

#include "Token.h"
#include "Node.h" // base class
#include "IConditionalStruct.h" // interface

namespace ndbl
{
    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "while"
     * while( condition_expr ) {
     *   // do something
     * }
     */
    class WhileLoopNode
        : public Node
        , public IConditionalStruct {
    public:
        WhileLoopNode();
        ~WhileLoopNode() = default;

        Token token_while;

        // implements IConditionalStruct (which is already documented)

        Property *       condition_property()const override { return props.get(k_conditional_cond_property_name);}
        Scope*           get_condition_true_scope()const override;
        Scope*           get_condition_false_scope()const override;
        void             set_cond_expr(InstructionNode*) override;
        InstructionNode* get_cond_expr()const override { return m_cond_instr_node; }

    private:
        InstructionNode* m_cond_instr_node;

        REFLECT_DERIVED_CLASS()
    };
}
