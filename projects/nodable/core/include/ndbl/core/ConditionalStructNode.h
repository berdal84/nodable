#pragma once

#include <memory>
#include <utility>
#include <ndbl/core/Token.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/IScope.h>
#include <ndbl/core/IConditionalStruct.h>

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

        void              set_token_if(token_ptr token);   // Set the "if" token (ex: { prefix: "", word: "if", suffix: " "})
        void              set_token_else(token_ptr token); // Set the "else" token (ex: { prefix: " ", word: "else", suffix: " "})
        token_cptr        get_token_if()const;             // Get the "if" token (ex: { prefix: "", word: "if", suffix: " "})
        token_cptr        get_token_else()const;           // Get the "else" token (ex: { prefix: " ", word: "else", suffix: " "})
        bool              has_elseif() const;              // Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        Scope*           get_condition_true_scope()const override;
        Scope*           get_condition_false_scope()const override;
        Property *       condition_property()const override;
        void             set_cond_expr(InstructionNode*) override;
        InstructionNode* get_cond_expr()const override;
    private:
        token_ptr        m_token_if;   // The "if" token (ex: { prefix: "", word: "if", suffix: " "})
        token_ptr        m_token_else; // The "else" token (ex: { prefix: " ", word: "else", suffix: " "})
        InstructionNode* m_cond_expr;  // The instruction to evaluate the condition

        REFLECT_DERIVED_CLASS(Node, IConditionalStruct)

    };
}