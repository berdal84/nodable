#pragma once

#include "Node.h"
#include "VariableNode.h"

namespace ndbl
{
    class Property;

    class VariableRefNode : public Node
    {
    public:

        ~VariableRefNode()
        {
            clear_variable();
        }

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

        inline void on_variable_name_change(const char* name)
        {
            m_value->token().word_replace( name );
        }

        void set_variable(VariableNode* variable)
        {
            VERIFY( m_variable == nullptr, "Can't call twice");

            m_variable = variable;
            m_value->set_type( m_variable->get_type() );
            m_value->token().word_replace( m_variable->get_identifier().c_str() );

            // bind signals
            CONNECT( m_variable->on_name_change, &VariableRefNode::on_variable_name_change );
            CONNECT( m_variable->on_destroy, &VariableRefNode::clear_variable );
        }

        void clear_variable()
        {
            if ( m_variable == nullptr )
                return;

            // unbind signals
            DISCONNECT(m_variable->on_destroy);
            DISCONNECT(m_variable->on_name_change);

            m_variable = nullptr;
        }

        inline const Token& get_identifier_token() const
        {
            return m_value->token(); // when parsed, this token may be a bit different from m_variable's (trailing ignored characters)
        }
    private:
        VariableNode* m_variable = nullptr;
    };
}