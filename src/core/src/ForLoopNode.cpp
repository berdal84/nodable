#include <nodable/core/ForLoopNode.h>
#include <nodable/core/Scope.h>

using namespace Nodable;

R_DEFINE_CLASS(ForLoopNode)

ForLoopNode::ForLoopNode()
        :
        m_token_for(nullptr)
{
    auto node_ptr_type = R::get_meta_type<Node*>();
    m_props.add(k_forloop_initialization_member_name , Visibility::Always, node_ptr_type, Way::Way_In);
    m_props.add(k_condition_member_name              , Visibility::Always, node_ptr_type, Way::Way_In);
    m_props.add(k_forloop_iteration_member_name      , Visibility::Always, node_ptr_type, Way::Way_In);
}

Scope* ForLoopNode::get_condition_true_branch() const
{
    return !m_successors.empty() ? m_successors[0]->get<Scope>() : nullptr;
}

Scope*  ForLoopNode::get_condition_false_branch() const
{
    return m_successors.size() > 1 ? m_successors[1]->get<Scope>() : nullptr;
}
