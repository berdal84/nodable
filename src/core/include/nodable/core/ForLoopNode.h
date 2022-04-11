#pragma once

#include <memory>
#include <nodable/core/Token.h>
#include <nodable/core/Node.h> // base class
#include <nodable/core/IConditionalStruct.h> // interface

namespace Nodable
{

    // forward declarations
    class Scope;
    class InstructionNode;

    /**
     * @brief Class to represent a conditional iterative structure: for( init_state, condition_expr, iterate_expr ) { ... }
     */
    class ForLoopNode
            : public Node
            , public IConditionalStruct {
    public:
        ForLoopNode();
        ~ForLoopNode() = default;

        inline void            set_token_for(std::shared_ptr<Token> _token) { m_token_for = _token; }
        inline std::shared_ptr<const Token> get_token_for()const   { return m_token_for; }
        Member*                get_init_expr()const { return m_props.get(k_forloop_initialization_member_name); }
        Member*                get_iter_expr()const { return m_props.get(k_forloop_iteration_member_name); }

        // override IConditionalStruct
        Member* condition_member()const override { return m_props.get(k_condition_member_name);}
        Scope*  get_condition_true_scope()const override;
        Scope*  get_condition_false_scope()const override;
        void set_iter_instr(InstructionNode*);
        void set_init_instr(InstructionNode*);
        void set_cond_instr(InstructionNode*) override;
        InstructionNode* get_init_instr()const { return m_init_instr_node; }
        InstructionNode* get_cond_instr()const override { return m_cond_instr_node; }
        InstructionNode* get_iter_instr()const { return m_iter_instr_node; }
    private:
        std::shared_ptr<Token> m_token_for;
        InstructionNode* m_init_instr_node;
        InstructionNode* m_cond_instr_node;
        InstructionNode* m_iter_instr_node;

        REFLECT_ENABLE(ForLoopNode, Node, IConditionalStruct)
    };
}
