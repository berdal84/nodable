#pragma once
#include <memory> // std::shared_ptr
#include "tools/core/reflection/reflection"
#include "Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        typedef tools::type type;
        Token               token;

        LiteralNode() {};
        ~LiteralNode() override {};

        void            init(const type* _type, const std::string& _name);
        Property*       value()       { ASSERT(m_value_property != nullptr); return m_value_property; }
        const Property* value() const { ASSERT(m_value_property != nullptr); return m_value_property; }
        Slot&           output_slot();
        const Slot&     output_slot() const;

    private:
        Property*       m_value_property = nullptr;
        const type*     m_type = nullptr;
        REFLECT_DERIVED_CLASS()
    };
}