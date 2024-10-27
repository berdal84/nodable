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
    m_value->set_type(_type);
    m_value->set_token({Token_t::identifier});
    m_value->token().word_replace(_identifier); // might come from std::string::c_str()

    // Init Slots
    add_slot(m_value, SlotFlag_INPUT, 1); // to connect an initialization expression
    add_slot(m_value, SlotFlag_FLOW_OUT, 1);
    add_slot(m_value, SlotFlag_FLOW_IN, Slot::MAX_CAPACITY);

    m_as_declaration_slot = add_slot(m_value, SlotFlag_OUTPUT, 1); // as declaration
    m_as_reference_slot   = add_slot(m_value, SlotFlag_OUTPUT, Slot::MAX_CAPACITY ); // as reference
}
