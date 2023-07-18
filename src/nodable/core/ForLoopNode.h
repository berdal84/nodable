#pragma once

#include <memory>
#include "Token.h"
#include "Node.h" // base class
#include "IConditionalStruct.h" // interface

namespace ndbl
{
    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @class Represent a conditional and iterative structure "for"
     * for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
        : public Node
        , public IConditionalStruct {
    public:
        ForLoopNode();
        ~ForLoopNode() = default;

        Token token_for;

        Property *       get_init_expr()const { return m_props.get(k_interative_init_property_name); }
        Property *       get_iter_expr()const { return m_props.get(k_interative_iter_property_name); }
        InstructionNode* get_iter_instr()const { return m_iter_instr_node; }
        InstructionNode* get_init_instr()const { return m_init_instr_node; }
        void             set_iter_instr(InstructionNode*);
        void             set_init_instr(InstructionNode*);

        // implements IConditionalStruct (which is already documented)

        Property *       condition_property()const override { return m_props.get(k_conditional_cond_property_name);}
        Scope*           get_condition_true_scope()const override;
        Scope*           get_condition_false_scope()const override;
        void             set_cond_expr(InstructionNode*) override;
        InstructionNode* get_cond_expr()const override { return m_cond_instr_node; }

    private:
        InstructionNode* m_init_instr_node;
        InstructionNode* m_cond_instr_node;
        InstructionNode* m_iter_instr_node;

        REFLECT_DERIVED_CLASS()
    };
}
