#pragma once
#include <nodable/Node.h>
#include <nodable/Reflect.h>

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(Reflect::Type type);
        ~LiteralNode() override = default;

        [[nodiscard]]
        inline Member* get_value() const { return m_props.get(Node::VALUE_MEMBER_NAME); }

        template<typename T>
        inline void set_value(T new_value) const { m_props.get(Node::VALUE_MEMBER_NAME)->set((T)new_value); }

        REFLECT_DERIVED(LiteralNode)
        REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}

