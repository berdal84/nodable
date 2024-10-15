#pragma once

#include "Node.h"
#include "VariableNode.h"

namespace ndbl
{
    class Property;

    class VariableRefNode : public Node
    {
    public:
        void init(const VariableNode* variable)
        {
            Node::init(NodeType_VARIABLE_REF, "");
            ASSERT(variable != nullptr)
            m_value = add_prop_copy( variable->get_value() );
            // Init Slots
            add_slot(SlotFlag_INPUT, 1,  m_value );
            add_slot(SlotFlag_PREV, Slot::MAX_CAPACITY );
        }
        const Property* get_value() const
        {
            return m_value;
        }
    private:
        Property* m_value = nullptr;
    };
}