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

        void set_variable(VariableNode* v)
        {
            m_value->set_type( v->get_type() );
            m_value->token().word_replace( v->get_identifier().c_str() );

            // Ensure this node name gets updated when variable's name changes
            v->on_name_change().connect([this](Node* _node) {
                auto name = static_cast<VariableNode*>( _node )->get_identifier();
                this->m_value->token().word_replace( name.c_str() );
            });
        }

        const Token& get_identifier_token() const
        {
            return m_value->token();
        }
    };
}