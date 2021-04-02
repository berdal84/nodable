#include "CodeBlockNode.h"
#include "Node/InstructionNode.h"

using namespace Nodable;

CodeBlockNode::CodeBlockNode()
        :
        AbstractCodeBlockNode()
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}

CodeBlockNode::~CodeBlockNode(){}

void CodeBlockNode::clear()
{
    // a code block do NOT owns its instructions nodes
    children.clear();
}

bool CodeBlockNode::hasInstructions() const
{
    return !children.empty();
}

InstructionNode* CodeBlockNode::getFirstInstruction() const
{
    return children.front()->as<InstructionNode>();
}
