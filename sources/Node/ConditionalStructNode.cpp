#include "ConditionalStructNode.h"

using namespace Nodable;

ConditionalStructNode::ConditionalStructNode()
    :
    CodeBlockNode(),
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    m_props.add("condition", Visibility::Always, Type_Boolean, Way::Way_In);
}

AbstractCodeBlockNode *ConditionalStructNode::getBranchTrue()
{
    return !m_children.empty() ? m_children[0]->as<AbstractCodeBlockNode>() : nullptr;
}

AbstractCodeBlockNode *ConditionalStructNode::getBranchFalse()
{
    return m_children.size() > 1 ? m_children[1]->as<AbstractCodeBlockNode>() : nullptr;
}

void ConditionalStructNode::getLastInstructions(std::vector<InstructionNode *>& out)
{
    if(auto branch_true = getBranchTrue())
        branch_true->getLastInstructions(out);
    if(auto branch_false = getBranchFalse())
        branch_false->getLastInstructions(out);
}
