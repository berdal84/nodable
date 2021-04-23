#include "ConditionalStructNode.h"

using namespace Nodable;

ConditionalStructNode::ConditionalStructNode()
    :
    CodeBlockNode(),
    m_token_if(nullptr),
    m_token_else(nullptr)
{
    props.add("condition", Visibility::Always, Type_Boolean, Way::Way_In);
}

AbstractCodeBlockNode *ConditionalStructNode::getNext()
{
    return (bool)*getCondition() ? getBranchTrue() : getBranchFalse();
}

AbstractCodeBlockNode *ConditionalStructNode::getBranchTrue()
{
    return !children.empty() ? children[0]->as<AbstractCodeBlockNode>() : nullptr;
}

AbstractCodeBlockNode *ConditionalStructNode::getBranchFalse()
{
    return children.size() > 1 ? children[1]->as<AbstractCodeBlockNode>() : nullptr;
}
