#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/Scope.h>
#include <nodable/core/reflection/R.h>

using namespace Nodable;

R_DEFINE_CLASS(ConditionalStructNode)

ConditionalStructNode::ConditionalStructNode()
    : Node()
    , m_token_if(nullptr)
    , m_token_else(nullptr)
{
    m_props.add<Node*>(k_condition_member_name, Visibility::Always, Way::Way_In);
}

Scope* ConditionalStructNode::get_condition_true_scope() const
{
    return !m_successors.empty() ? m_successors[0]->get<Scope>() : nullptr;
}

Scope* ConditionalStructNode::get_condition_false_scope() const
{
    return m_successors.size() > 1 ? m_successors[1]->get<Scope>() : nullptr;
}
