#include "VariableNode.h"
#include "Property.h"
#include "Scope.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<VariableNode>("VariableNode").extends<Node>();
}

void VariableNode::init(const tools::TypeDescriptor* _type, const char* _identifier)
{
    // Init node
    Node::init(NodeType_VARIABLE, "Variable");

    // Init identifier property
    m_identifier = add_prop(_type, VALUE_PROPERTY );
    m_identifier->set_token({Token_t::identifier});
    m_identifier->get_token().word_replace(_identifier); // might come from std::string::c_str()

    // Init Slots
    add_slot(SlotFlag_INPUT, 1, m_identifier); // to connect an initialization expression
    add_slot(SlotFlag_OUTPUT, Slot::MAX_CAPACITY, m_identifier); // can be connected by reference
    // add_slot(SlotFlag_OUTPUT, Slot::MAX_CAPACITY, m_value);   // CANNOT be connected by value
    add_slot(SlotFlag_OUTPUT, 1, m_this_as_property );
    add_slot(SlotFlag_PREV, Slot::MAX_CAPACITY );
}

Scope* VariableNode::get_scope()
{
    if (m_scope != nullptr)
        return m_scope->get_component<Scope>(); // TODO: Scope should NOT be a Node, that's a nonsense.
    return {};
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
    ASSERT(m_identifier != nullptr)
    return m_identifier;
}

const Property* VariableNode::get_value() const
{
    ASSERT(m_identifier != nullptr)
    return m_identifier;
}

Slot& VariableNode::input_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->input_slot() );
}

const Slot& VariableNode::input_slot() const
{
    return *find_slot_by_property(m_identifier, SlotFlag_INPUT );
}

Slot& VariableNode::output_slot()
{
    return const_cast<Slot&>( const_cast<const VariableNode*>(this)->output_slot() );
}

const Slot& VariableNode::output_slot() const
{
    return *find_slot_by_property(m_identifier, SlotFlag_OUTPUT );
}

