#pragma once

#include "Node.h"
#include "VariableNode.h"

namespace ndbl
{
    class Property;

    class VariableRefNode : public Node
    {
    public:

        void init()
        {
            Node::init(NodeType_VARIABLE_REF, "");

            // Set name
            //set_name("Ref.");

            // Init identifier property
            m_value->set_type(tools::type::any());
            m_value->set_token({Token_t::identifier});

            // Init Slots
            add_slot(m_value, SlotFlag_PARENT, 1);
            add_slot(m_value, SlotFlag_NEXT, 1);
            add_slot(m_value, SlotFlag_PREV, Slot::MAX_CAPACITY);
            add_slot(m_value, SlotFlag_INPUT, 1);
            add_slot(m_value, SlotFlag_OUTPUT, 1); // ref can be connected once
        }

        void on_variable_name_change(const char* name)
        {
            m_value->token().word_replace( name );
        }

        void set_variable(VariableNode* variable)
        {
            m_value->set_type( variable->get_type() );
            m_value->token().word_replace(variable->get_identifier().c_str() );

            // Ensure this node name gets updated when variable's name changes
            CONNECT( variable->on_name_change_signal, VariableRefNode::on_variable_name_change);
        }

        const Token& get_identifier_token() const
        {
            return m_value->token();
        }
    };
}