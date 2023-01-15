#pragma once

#include <memory>
#include <utility>
#include <nodable/core/Token.h>
#include <nodable/core/Node.h>
#include <nodable/core/IScope.h>
#include <nodable/core/IConditionalStruct.h>

namespace ndbl
{

    /**
     * @class Represent a conditional structure with two branches ( IF/ELSE )
     * @note Multiple ConditionalStructNode can be chained to form an IF / ELSE IF / ... / ELSE.
     */
    class ConditionalStructNode
        : public Node
        , public IConditionalStruct
    {
        using token_ptr  = std::shared_ptr<Token>;
        using token_cptr = std::shared_ptr<const Token>;
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;

        inline void       set_token_if(token_ptr token) { m_token_if = std::move(token); }
        inline void       set_token_else(token_ptr token) { m_token_else = std::move(token); }
        inline token_cptr get_token_if()const   { return m_token_if; }
        inline token_cptr get_token_else()const { return m_token_else; }
        bool              has_elseif() const; // Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        Scope*           get_condition_true_scope()const override;
        Scope*           get_condition_false_scope()const override;
        Property *       condition_property()const override { return m_props.get(k_conditional_cond_property_name); }
        void             set_cond_expr(InstructionNode*) override;
        InstructionNode* get_cond_expr()const override { return m_cond_expr; }
    private:
        token_ptr        m_token_if;
        token_ptr        m_token_else;
        InstructionNode* m_cond_expr;

        REFLECT_DERIVED_CLASS(Node, IConditionalStruct)

    };
}