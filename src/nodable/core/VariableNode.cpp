#include "VariableNode.h"
#include "Property.h"
#include "Scope.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<VariableNode>("VariableNode").extends<Node>();
}


VariableNode::VariableNode()
: Node("Variable")
, m_is_declared(false)
, m_type(fw::type::any())
{
}

VariableNode::VariableNode(const fw::type *_type, const char* identifier)
: Node("Variable")
, identifier_token( Token_t::identifier )
, m_is_declared( false )
, m_type( _type )
{
    set_name(identifier);
}

void VariableNode::init()
{
    Node::init();

    m_value_property_id = add_prop( m_type, VALUE_PROPERTY, PropertyFlag_DEFAULT );
    add_slot( SlotFlag_INPUT,  1, m_value_property_id);
    add_slot( SlotFlag_OUTPUT, SLOT_MAX_CAPACITY, m_value_property_id);
    add_slot( SlotFlag_OUTPUT, 1, m_this_property_id );

    add_slot( SlotFlag_PREV, SLOT_MAX_CAPACITY );
}

PoolID<Scope> VariableNode::get_scope()
{
    Node* scope_node = m_scope.get();
    return scope_node ? scope_node->get_component<Scope>() : PoolID<Scope>{};
}

const fw::type *VariableNode::type() const
{
    return property()->get_type();
}

fw::variant* VariableNode::value()
{
    return property()->value();
}

void VariableNode::reset_scope(Scope* _scope)
{
    m_scope = _scope ? _scope->get_owner() : PoolID<Node>{};
}

Property *VariableNode::property()
{
    Property* p = get_prop_at( m_value_property_id );
    FW_ASSERT(p != nullptr)
    return p;
}

const Property* VariableNode::property() const
{
    const Property* p = get_prop_at( m_value_property_id );
    FW_ASSERT(p != nullptr)
    return p;
}

Slot& VariableNode::input_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->input_slot() );
}

const Slot& VariableNode::input_slot() const
{
    return *find_slot_by_property_id( m_value_property_id, SlotFlag_INPUT );
}

Slot& VariableNode::output_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->output_slot() );
}

const Slot& VariableNode::output_slot() const
{
    return *find_slot_by_property_id( m_value_property_id, SlotFlag_OUTPUT );
}