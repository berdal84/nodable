#include <nodable/core/Log.h>
#include <nodable/core/Property.h>
#include <nodable/core/VariableNode.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<VariableNode>("VariableNode").extends<Node>();
}


VariableNode::VariableNode(const type& _type, const char* identifier)
    : Node("Variable")
    , m_declaration_instr(nullptr)
    , m_type_token(nullptr)
    , m_identifier_token(std::make_shared<Token>(Token_t::identifier, "", 0)) // unnamed by default
    , m_assignment_operator_token(nullptr)                                    // unassigned by default
    , m_is_declared(false)
    , m_scope(nullptr)
{
    set_name(identifier);
	m_value = m_props.add(_type, k_value_property_name, Visibility::Always, Way_InOut).get();
}

void VariableNode::set_name(const char* identifier)
{
    m_identifier_token->set_word(identifier);
    Node::set_name(identifier);
}
