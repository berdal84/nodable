#pragma once
#include <nodable/core/Node.h>
#include <nodable/core/reflection/type.>
#include <memory> // std::shared_ptr

namespace Nodable
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(type);
        ~LiteralNode() override = default;

        [[nodiscard]]
        inline Member* get_value() const { return m_props.get(k_value_member_name); }

        template<typename T>
        inline void set_value(T _value) const { m_props.get(k_value_member_name)->set(_value); }

        REFLECT_DERIVED_CLASS(Node)
    };
}

