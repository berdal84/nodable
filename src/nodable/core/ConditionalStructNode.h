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
        ConditionalStructNode(ConditionalStructNode&& other) = default;
        ~ConditionalStructNode() = default;
        ConditionalStructNode& operator=(ConditionalStructNode&&) = default;

        Token               token_if;   // The "if" token (ex: { prefix: "", word: "if", suffix: " "})
        Token               token_else; // The "else" token (ex: { prefix: " ", word: "else", suffix: " "})
        ID<InstructionNode> cond_expr;  // The instruction to evaluate the condition

        bool              has_elseif() const;// Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        ID<Scope>       get_condition_true_scope()const override;
        ID<Scope>       get_condition_false_scope()const override;
        const Property* condition_property()const override;

        REFLECT_DERIVED_CLASS()
    };
}


static_assert(std::is_move_assignable_v<ndbl::ConditionalStructNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::ConditionalStructNode>, "Should be move constructible");
