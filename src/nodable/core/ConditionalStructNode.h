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
        bool is_chained_with_other_cond_struct() const; // Check if another conditional structure is connected to the else branch (forming an else if)

        // implement IConditionalStruct (which is already documented)

        PoolID<Scope>  get_scope_at(size_t _pos) const override;
        Slot&          get_child_slot_at(size_t _pos) override;
        const Slot&    get_child_slot_at(size_t _pos) const override;

    private:
        std::array<ID8<Slot>, 2> m_next_slot;
        std::array<ID8<Slot>, 2> m_child_slot;
    };
}


static_assert(std::is_move_assignable_v<ndbl::ConditionalStructNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::ConditionalStructNode>, "Should be move constructible");
