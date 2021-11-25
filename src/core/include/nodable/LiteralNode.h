#pragma once
#include <nodable/Node.h>

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(Type type);
        ~LiteralNode() override = default;

        [[nodiscard]] inline Member* value() const { return m_props.get("value"); }

        REFLECT_WITH_INHERITANCE(LiteralNode)
        REFLECT_INHERITS(Node)
        REFLECT_END
    };
}

