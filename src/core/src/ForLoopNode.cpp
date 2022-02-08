#include <nodable/ForLoopNode.h>
#include <nodable/Scope.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(ForLoopNode)

ForLoopNode::ForLoopNode()
        :
        m_token_for(nullptr)
{
    m_props.add("init"     , Visibility::Always, Reflect::Type_Any        , Way::Way_In);
    m_props.add("condition", Visibility::Always, Reflect::Type_Object_Ptr , Way::Way_In);
    m_props.add("iter"     , Visibility::Always, Reflect::Type_Any        , Way::Way_In);
}

Scope* ForLoopNode::get_condition_true_branch() const
{
    return !m_next.empty() ? m_next[0]->get<Scope>() : nullptr;
}

Scope*  ForLoopNode::get_condition_false_branch() const
{
    return m_next.size() > 1 ? m_next[1]->get<Scope>() : nullptr;
}
