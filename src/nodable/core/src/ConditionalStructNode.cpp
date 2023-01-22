#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/Scope.h>

#include <fw/reflection/reflection>

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<ConditionalStructNode>("ConditionalStructNode")
        .extends<Node>()
        .extends<IConditionalStruct>();
}

ConditionalStructNode::ConditionalStructNode()
    : Node()
    , m_token_if(nullptr)
    , m_token_else(nullptr)
{
    m_props.add<Node*>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In);
}

Scope* ConditionalStructNode::get_condition_true_scope() const
{
    return !m_successors.empty() ? m_successors[0]->get<Scope>() : nullptr;
}

Scope* ConditionalStructNode::get_condition_false_scope() const
{
    return m_successors.size() > 1 ? m_successors[1]->get<Scope>() : nullptr;
}

void ConditionalStructNode::set_cond_expr(InstructionNode* _node)
{
    m_cond_expr = _node;
}

bool ConditionalStructNode::has_elseif() const
{
    Node* false_node = m_successors.size() > 1 ? m_successors[1] : nullptr;
    return false_node && false_node->is<ConditionalStructNode>();
}

void ConditionalStructNode::set_token_if(ConditionalStructNode::token_ptr token)
{
    m_token_if = std::move(token);
}

void ConditionalStructNode::set_token_else(ConditionalStructNode::token_ptr token)
{
    m_token_else = std::move(token);
}

ConditionalStructNode::token_cptr ConditionalStructNode::get_token_if() const
{
    return m_token_if;
}

ConditionalStructNode::token_cptr ConditionalStructNode::get_token_else() const
{
    return m_token_else;
}
Property *ConditionalStructNode::condition_property() const
{
    return m_props.get(k_conditional_cond_property_name);
}

InstructionNode *ConditionalStructNode::get_cond_expr() const
{
    return m_cond_expr;
}
