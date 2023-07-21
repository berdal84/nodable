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
        Property* root;
        Token     token_end;

        explicit InstructionNode();
        ~InstructionNode()= default;

        REFLECT_DERIVED_CLASS()
    };
}
