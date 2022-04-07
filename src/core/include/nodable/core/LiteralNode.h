#pragma once
#include <nodable/core/Node.h>
#include <nodable/core/reflection/R.h>
#include <memory> // std::shared_ptr

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(std::shared_ptr<const R::Meta_t>);
        ~LiteralNode() override = default;

        [[nodiscard]]
        inline Member* get_value() const { return m_props.get(k_value_member_name); }

        template<typename T>
        inline void set_value(T _value) const { m_props.get(k_value_member_name)->set(_value); }

        R_DERIVED(LiteralNode)
        R_EXTENDS(Node)
        R_END
    };
}

