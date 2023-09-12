#pragma once

#include <string>
#include <memory>

#include "fw/core/types.h"
#include "core/Node.h" // base class
#include "core/Property.h"


namespace ndbl
{
    // forward declarations
    struct Token;

    class InstructionNode : public Node
    {
        REFLECT_DERIVED_CLASS()
    public:
        Token     token_end;

        InstructionNode() = default;
        InstructionNode(InstructionNode&& other) = default;
        ~InstructionNode()= default;
        InstructionNode& operator=(InstructionNode&& other) = default;

        void            init() override;
        Slot&           root_slot();
        const Slot&     root_slot() const;

    private:
        ID<Slot> m_root_slot_id;
    };
}

static_assert(std::is_move_assignable_v<ndbl::InstructionNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::InstructionNode>, "Should be move constructible");
