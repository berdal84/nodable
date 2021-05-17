#include <nodable/ConditionalStructNode.h>
#include <nodable/ScopedCodeBlockNode.h>

using namespace Nodable::core;

ConditionalStructNode::ConditionalStructNode()
    :
    CodeBlockNode(),
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, Type_Boolean, Way::Way_In);
    setNextMaxCount(2); // allow 2 Nodes to be next.
}

ScopedCodeBlockNode *ConditionalStructNode::getBranchTrue() const
{
    return !m_next.empty() ? m_next[0]->as<ScopedCodeBlockNode>() : nullptr;
}

ScopedCodeBlockNode *ConditionalStructNode::getBranchFalse() const
{
    return m_next.size() > 1 ? m_next[1]->as<ScopedCodeBlockNode>() : nullptr;
}

void ConditionalStructNode::getLastInstructions(std::vector<InstructionNode *>& out)
{
    if(auto branch_true = getBranchTrue())
        branch_true->getLastInstructions(out);
    if(auto branch_false = getBranchFalse())
        branch_false->getLastInstructions(out);
}
