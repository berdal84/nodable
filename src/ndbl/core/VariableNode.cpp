#include "VariableNode.h"
#include "Property.h"
#include "Scope.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<VariableNode>("VariableNode").extends<Node>();
}

void VariableNode::init(const tools::type* _val_type, const char* _identifier)
{
    // Init node
    Node::init(NodeType_VARIABLE, _identifier);
    identifier_token = Token_t::identifier; // has an identifier!

    // Init value property
    m_value = add_prop(_val_type, VALUE_PROPERTY );

    // Init Slots
    add_slot(SlotFlag_INPUT, 1, m_value);
    add_slot(SlotFlag_OUTPUT, Slot::MAX_CAPACITY, m_value);
    add_slot(SlotFlag_OUTPUT, 1, m_this_as_property );
    add_slot(SlotFlag_PREV, Slot::MAX_CAPACITY );
}

Scope* VariableNode::get_scope()
{
    if (m_scope != nullptr)
        m_scope->get_component<Scope>();
    return {};
}

const type *VariableNode::get_value_type() const
{
    return get_value()->get_type();
}

void VariableNode::reset_scope(Scope* _scope)
{
    if( _scope != nullptr )
        m_scope = _scope->get_owner();
    else
        m_scope = {};
}

Property* VariableNode::property()
{
    ASSERT(m_value != nullptr)
    return m_value;
}

const Property* VariableNode::get_value() const
{
    ASSERT(m_value != nullptr)
    return m_value;
}

Slot& VariableNode::input_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->input_slot() );
}

const Slot& VariableNode::input_slot() const
{
    return *find_slot_by_property(m_value, SlotFlag_INPUT );
}

Slot& VariableNode::output_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->output_slot() );
}

const Slot& VariableNode::output_slot() const
{
    return *find_slot_by_property(m_value, SlotFlag_OUTPUT );
}

