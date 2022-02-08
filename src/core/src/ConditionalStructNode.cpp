#include <nodable/ConditionalStructNode.h>
#include <nodable/Scope.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(ConditionalStructNode)

ConditionalStructNode::ConditionalStructNode()
    :
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, Reflect::Type_Object_Ptr, Way::Way_In);
}

Scope* ConditionalStructNode::get_condition_true_branch() const
{
    return !m_next.empty() ? m_next[0]->get<Scope>() : nullptr;
}

Scope* ConditionalStructNode::get_condition_false_branch() const
{
    return m_next.size() > 1 ? m_next[1]->get<Scope>() : nullptr;
}
