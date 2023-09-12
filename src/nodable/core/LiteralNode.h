#pragma once
#include <memory> // std::shared_ptr
#include "fw/core/reflection/reflection"
#include "core/Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:

        LiteralNode() = default;
        explicit LiteralNode(const fw::type*);
        LiteralNode(LiteralNode&&) = default;
        LiteralNode& operator=(LiteralNode&&) = default;
        ~LiteralNode() override = default;

        void            init() override;
        Property*       value() { return get_prop_at( m_value_property_id ); }
        const Property* value() const { return get_prop_at( m_value_property_id ); }
    private:
        ID<Property>    m_value_property_id;
        const fw::type* m_type;
        REFLECT_DERIVED_CLASS()
    };
}

