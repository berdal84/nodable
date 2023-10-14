#include "VariableNode.h"
#include "Property.h"
#include "InstructionNode.h"
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

    add_slot( SlotFlag_PARENT,  1, m_this_property_id );
    add_slot( SlotFlag_OUTPUT, 1, m_this_property_id );
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

Slot* VariableNode::find_value_typed_slot( SlotFlags _flags)
{
    return find_slot( m_value_property_id, _flags & SlotFlag_ORDER_MASK | SlotFlag_TYPE_VALUE );
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

const Slot& VariableNode::input_slot() const
{
    return *find_slot( m_value_property_id, SlotFlag_INPUT );
}
