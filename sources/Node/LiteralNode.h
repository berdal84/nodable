#pragma once
#include "Node/Node.h"

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(Type type): Node("Literal")
        {
            m_props.add("value", Visibility::Always, type, Way_Out);
        };
        ~LiteralNode() override = default;

        [[nodiscard]] inline Member* value() const { return m_props.get("value"); }

    MIRROR_CLASS(LiteralNode)(MIRROR_PARENT(Node))
    };
}

