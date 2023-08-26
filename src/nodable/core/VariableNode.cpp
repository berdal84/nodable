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
{
}

VariableNode::VariableNode(const fw::type *_type, const char* identifier)
    : Node("Variable")
    , identifier_token(Token_t::identifier)
    , m_is_declared(false)
{
    set_name(identifier);
    m_value = props.add(_type, k_value_property_name, Visibility::Always, Way_InOut);
}

ID<Scope> VariableNode::get_scope()
{
    Node* scope_node = m_scope.get();
    return scope_node ? scope_node->get_component<Scope>() : ID<Scope>{};
}

const fw::type *VariableNode::type() const
{
    return m_value->get_type();
}

fw::variant* VariableNode::value()
{
    return m_value->value();
}

void VariableNode::reset_scope(Scope* _scope)
{
    m_scope = _scope ? _scope->get_owner() : ID<Node>{};
}
