#pragma once

#include "ASTNode.h"
#include "ASTVariable.h"

namespace ndbl
{
    class ASTNodeProperty;

    class ASTVariableRef : public ASTNode
    {
    public:

        ~ASTVariableRef()
        {
            clear_variable();
        }

        void init()
        {
            ASTNode::init(ASTNodeType_VARIABLE_REF, "");

            // Set name
            //set_name("Ref.");

            // Init identifier property
            m_value->set_type(tools::type::any());
            m_value->set_token({ASTToken_t::identifier});

            // Init Slots
            add_slot(m_value, SlotFlag_FLOW_OUT, 1);
            add_slot(m_value, SlotFlag_FLOW_IN , ASTNodeSlot::MAX_CAPACITY);
            add_slot(m_value, SlotFlag_INPUT   , 1);
            add_slot(m_value, SlotFlag_OUTPUT  , 1); // ref can be connected once
        }

        void on_variable_name_change(const std::string& name)
        {
            m_value->token().word_replace( name.c_str() );
        }

        void set_variable(ASTVariable* variable)
        {
            VERIFY( m_variable == nullptr, "Can't call twice");

            m_variable = variable;
            m_value->set_type( m_variable->get_type() );
            m_value->token().word_replace( m_variable->get_identifier().c_str() );

            // bind signals
            CONNECT(m_variable->on_name_change, &ASTVariableRef::on_variable_name_change, this );
            CONNECT(m_variable->on_shutdown, &ASTVariableRef::clear_variable, this );
        }

        void clear_variable()
        {
            if ( m_variable == nullptr )
                return;

            // unbind signals
            DISCONNECT(m_variable->on_shutdown, this);
            DISCONNECT(m_variable->on_name_change, this);

            m_variable = nullptr;
        }

        inline const ASTToken& get_identifier_token() const
        {
            return m_value->token(); // when parsed, this token may be a bit different from m_variable's (trailing ignored characters)
        }
    private:
        ASTVariable* m_variable = nullptr;
    };
}