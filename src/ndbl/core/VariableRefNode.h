#pragma once

#include "Node.h"
#include "VariableNode.h"

namespace ndbl
{
    class Property;

    class VariableRefNode : public Node
    {
    public:
        void init(const tools::TypeDescriptor* _type)
        {
            Node::init(NodeType_VARIABLE_REF, "");
            ASSERT(_type != nullptr)

            // Set name
            set_name("Ref.");

            // Init identifier property
            Property* new_value = add_prop(_type, VALUE_PROPERTY );
            new_value->set_token({Token_t::identifier});

            // Init Slots
            m_input_slot = add_slot(SlotFlag_INPUT, 1, new_value );
            add_slot(SlotFlag_PREV, Slot::MAX_CAPACITY );
        }

        Slot* get_input_slot()
        {
            return m_input_slot;
        }

        const Token& get_identifier_token() const
        {
            const Slot* adjacent_slot = m_input_slot->first_adjacent();
            const VariableNode* variable = static_cast<const VariableNode*>( adjacent_slot->get_node() );
            return variable->get_identifier_token(); // This node's m_value's token cannot be edited, plus we have to return VariableNode's (may have been changed)
        }

    private:
        Slot* m_input_slot = nullptr; // The property connected to the m_variable's value Property.
    };
}