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
        REFLECT_DERIVED_CLASS()
    public:
        Token token_if;   // Example: { prefix: "", word: "if", suffix: " "}
        Token token_else; // Example: { prefix: " ", word: "else", suffix: " "}
        PoolID<InstructionNode> cond_expr; // The instruction to evaluate the condition

        ConditionalStructNode() = default;
        ConditionalStructNode(ConditionalStructNode&& other) = default;
        ~ConditionalStructNode() = default;
        ConditionalStructNode& operator=(ConditionalStructNode&&) = default;

        void init() override;
        bool is_chain() const; // Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        PoolID<Scope>   get_condition_true_scope()const override;
        PoolID<Scope>   get_condition_false_scope()const override;
        const Property* condition_property()const override;
    };
}


static_assert(std::is_move_assignable_v<ndbl::ConditionalStructNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::ConditionalStructNode>, "Should be move constructible");
