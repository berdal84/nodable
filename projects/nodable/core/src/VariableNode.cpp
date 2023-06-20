#include <ndbl/core/VariableNode.h>

#include "fw/core/log.h"
#include <ndbl/core/Property.h>

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<VariableNode>("VariableNode").extends<Node>();
}


VariableNode::VariableNode(const fw::type& _type, const char* identifier)
    : Node("Variable")
    , m_declaration_instr(nullptr)
    , identifier_token(Token_t::identifier)
    , m_is_declared(false)
    , m_scope(nullptr)
{
    set_name(identifier);
	m_value = m_props.add(_type, k_value_property_name, Visibility::Always, Way_InOut).get();
}