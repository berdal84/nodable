#pragma once
#include <memory> // std::shared_ptr
#include "tools/core/reflection/reflection"
#include "Node.h" // Base class

namespace ndbl
{
    class LiteralNode: public Node
    {
    public:
        using type = tools::type;

        LiteralNode() = default;
        explicit LiteralNode(const tools::type*);
        ~LiteralNode() override = default;

        void            init() override;
        Property*       value();
        const Property* value() const;
    private:
        Property*   m_value_property;
        const type* m_type;
        REFLECT_DERIVED_CLASS()
    };
}

