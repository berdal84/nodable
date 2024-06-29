#include "VariableNode.h"
#include "Property.h"
#include "Scope.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<VariableNode>("VariableNode").extends<Node>();
}


VariableNode::VariableNode()
: Node("Variable")
{}

VariableNode::VariableNode(const tools::type *_type, const char* identifier)
: Node("Variable")
, identifier_token( Token_t::identifier )
, m_type( _type )
{
    set_name(identifier);
}

void VariableNode::init()
{
    Node::init();

    m_value_property = add_prop(m_type, VALUE_PROPERTY, PropertyFlag_DEFAULT );
    add_slot(SlotFlag_INPUT, 1, m_value_property);
    add_slot(SlotFlag_OUTPUT, Slot::MAX_CAPACITY, m_value_property);
    add_slot(SlotFlag_OUTPUT, 1, m_this_as_property );

    add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY );
}

Scope* VariableNode::get_scope()
{
    if (m_scope != nullptr)
        m_scope->get_component<Scope>();
    return {};
}

const type *VariableNode::get_value_type() const
{
    return property()->get_type();
}

variant* VariableNode::get_value()
{
    return property()->value();
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
    ASSERT(m_value_property != nullptr)
    return m_value_property;
}

const Property* VariableNode::property() const
{
    ASSERT(m_value_property != nullptr)
    return m_value_property;
}

Slot& VariableNode::input_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->input_slot() );
}

const Slot& VariableNode::input_slot() const
{
    return *find_slot_by_property(m_value_property, SlotFlag_INPUT );
}

Slot& VariableNode::output_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->output_slot() );
}

const Slot& VariableNode::output_slot() const
{
    return *find_slot_by_property(m_value_property, SlotFlag_OUTPUT );
}

