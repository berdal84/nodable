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
    public:
        Token     token_end;

        InstructionNode();
        InstructionNode(InstructionNode&& other) = default;
        ~InstructionNode()= default;
        InstructionNode& operator=(InstructionNode&& other) = default;
        const Property* root_property() const { return get_prop(VALUE_PROPERTY); }
        Slot root_slot() const { return get_slot(VALUE_PROPERTY, Way::In); }
        REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::InstructionNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::InstructionNode>, "Should be move constructible");
