#pragma once

#include <memory>
#include <utility>
#include "Token.h"
#include "Node.h"
#include "IScope.h"
#include "IConditionalStruct.h"

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
    public:
        ConditionalStructNode();
        ~ConditionalStructNode() = default;
        Token token_if;   // The "if" token (ex: { prefix: "", word: "if", suffix: " "})
        Token token_else; // The "else" token (ex: { prefix: " ", word: "else", suffix: " "})

        bool              has_elseif() const;              // Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        Scope*           get_condition_true_scope()const override;
        Scope*           get_condition_false_scope()const override;
        Property *       condition_property()const override;
        void             set_cond_expr(InstructionNode*) override;
        InstructionNode* get_cond_expr()const override;
    private:
        InstructionNode* m_cond_expr;  // The instruction to evaluate the condition

        REFLECT_DERIVED_CLASS()
    };
}