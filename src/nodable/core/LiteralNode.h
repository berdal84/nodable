#pragma once
#include <memory> // std::shared_ptr
#include "fw/core/reflection/reflection"
#include "core/Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        Property* value;

        explicit LiteralNode(const fw::type*);
        ~LiteralNode() override = default;

        REFLECT_DERIVED_CLASS()
    };
}

