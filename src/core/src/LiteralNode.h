#pragma once
#include "Node.h"

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(Type type);;
        ~LiteralNode() override = default;

        [[nodiscard]] inline Member* value() const { return m_props.get("value"); }

    MIRROR_CLASS(LiteralNode)(MIRROR_PARENT(Node))
    };
}

