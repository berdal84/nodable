#pragma once
#include <nodable/core/Node.h>
#include <fw/reflection/reflection>
#include <memory> // std::shared_ptr

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        explicit LiteralNode(fw::type);
        ~LiteralNode() override = default;

        [[nodiscard]]
        inline Property * get_value() const { return m_props.get(k_value_property_name); }

        template<typename T>
        inline void set_value(T _value) const { m_props.get(k_value_property_name)->set(_value); }

        REFLECT_DERIVED_CLASS(Node)
    };
}

