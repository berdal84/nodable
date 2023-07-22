#include "ConditionalStructNode.h"
#include "fw/core/reflection/reflection"
#include "Scope.h"

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
    , m_cond_expr(nullptr)
{
    props.add<Node*>(k_conditional_cond_property_name, Visibility::Always, Way::Way_In);
}

Scope* ConditionalStructNode::get_condition_true_scope() const
{
    return !successors.empty() ? successors[0]->components.get<Scope>() : nullptr;
}

Scope* ConditionalStructNode::get_condition_false_scope() const
{
    return successors.size() > 1 ? successors[1]->components.get<Scope>() : nullptr;
}

void ConditionalStructNode::set_cond_expr(InstructionNode* _node)
{
    m_cond_expr = _node;
}

bool ConditionalStructNode::has_elseif() const
{
    Node* false_node = successors.size() > 1 ? successors[1] : nullptr;
    return false_node && fw::extends<ConditionalStructNode>(false_node);
}

Property *ConditionalStructNode::condition_property() const
{
    return props.get(k_conditional_cond_property_name);
}

InstructionNode *ConditionalStructNode::get_cond_expr() const
{
    return m_cond_expr;
}
