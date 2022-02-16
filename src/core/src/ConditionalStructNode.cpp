#include <nodable/ConditionalStructNode.h>
#include <nodable/Scope.h>
#include <nodable/R.h>

using namespace Nodable;

R_DEFINE_CLASS(ConditionalStructNode)

ConditionalStructNode::ConditionalStructNode()
    : Node()
    , m_token_if(nullptr)
    , m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, R::add_ptr(R::Type::Void), Way::Way_In);
}

Scope* ConditionalStructNode::get_condition_true_branch() const
{
    return !m_successors.empty() ? m_successors[0]->get<Scope>() : nullptr;
}

Scope* ConditionalStructNode::get_condition_false_branch() const
{
    return m_successors.size() > 1 ? m_successors[1]->get<Scope>() : nullptr;
}
