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
            ASSERT(_type != nullptr)
            Node::init(NodeType_VARIABLE_REF, "");

            // Set name
            //set_name("Ref.");

            // Init identifier property
            m_value->set_type(_type);
            m_value->set_token({Token_t::identifier});

            // Init Slots
            add_slot(m_value, SlotFlag_PARENT, 1);
            add_slot(m_value, SlotFlag_NEXT, 1);
            add_slot(m_value, SlotFlag_PREV, Slot::MAX_CAPACITY);
            add_slot(m_value, SlotFlag_INPUT, 1);
            add_slot(m_value, SlotFlag_OUTPUT, 1); // ref can be connected once
        }

        const Token& get_identifier_token() const
        {
            const Slot* adjacent_slot = value_in()->first_adjacent();

            if ( adjacent_slot == nullptr )
                return value()->token(); // In some cases ref are not connected to an existing variable

            return adjacent_slot->node()->value()->token();
        }
    };
}