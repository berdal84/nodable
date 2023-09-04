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
        Property* value() { return get_prop_at( m_value_property_id ); }
        const Property* value() const { return get_prop_at( m_value_property_id ); }
    private:
        size_t m_value_property_id;

        REFLECT_DERIVED_CLASS()
    };
}

