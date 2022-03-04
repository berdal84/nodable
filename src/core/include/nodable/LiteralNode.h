#pragma once
#include <nodable/Node.h>
#include <nodable/R.h>
#include <memory> // std::shared_ptr

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(std::shared_ptr<const R::Type>);
        ~LiteralNode() override = default;

        [[nodiscard]]
        inline Member* get_value() const { return m_props.get(Node::VALUE_MEMBER_NAME); }

        template<typename T>
        inline void set_value(T new_value) const { m_props.get(Node::VALUE_MEMBER_NAME)->set((T)new_value); }

        R_DERIVED(LiteralNode)
        R_EXTENDS(Node)
        R_END
    };
}

