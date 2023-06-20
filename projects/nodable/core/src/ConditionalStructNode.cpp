#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/Scope.h>

#include <fw/core/reflection/reflection>

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

Property *ConditionalStructNode::condition_property() const
{
    return m_props.get(k_conditional_cond_property_name);
}

InstructionNode *ConditionalStructNode::get_cond_expr() const
{
    return m_cond_expr;
}
