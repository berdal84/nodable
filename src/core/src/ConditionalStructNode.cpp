#include <nodable/ConditionalStructNode.h>
#include <nodable/ScopeNode.h>

using namespace Nodable;

REFLECT_DEFINE(ConditionalStructNode)

ConditionalStructNode::ConditionalStructNode()
    :
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, Type_Boolean, Way::Way_In);
    setPrevMaxCount(1); // allow 1 Nodes to be previous.
    setNextMaxCount(2); // allow 2 Nodes to be next (branches if and else).
}

ScopeNode *ConditionalStructNode::get_condition_true_branch() const
{
    return !m_next.empty() ? m_next[0]->as<ScopeNode>() : nullptr;
}

ScopeNode *ConditionalStructNode::get_condition_false_branch() const
{
    return m_next.size() > 1 ? m_next[1]->as<ScopeNode>() : nullptr;
}

void ConditionalStructNode::get_last_instructions(std::vector<InstructionNode *> &out)
{
    if (get_condition_true_branch() )
    {
        get_condition_true_branch()->get_last_instructions(out);
    }
    if (get_condition_false_branch() )
    {
        get_condition_false_branch()->get_last_instructions(out);
    }
}