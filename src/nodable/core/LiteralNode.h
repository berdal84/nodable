#pragma once
#include <memory> // std::shared_ptr
#include "fw/core/reflection/reflection"
#include "core/Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:

        LiteralNode();
        LiteralNode(LiteralNode&&) = default;
        LiteralNode& operator=(LiteralNode&&) = default;
        explicit LiteralNode(const fw::type*);
        ~LiteralNode() override = default;
        Property* value() const { return m_value; }
    private:
        Property* m_value;

        REFLECT_DERIVED_CLASS()
    };
}

