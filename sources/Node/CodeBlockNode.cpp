#include "CodeBlockNode.h"
#include "Node/InstructionNode.h"

using namespace Nodable;

CodeBlockNode::~CodeBlockNode()
{
    CodeBlockNode::clear();
}

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


CodeBlockNode::CodeBlockNode()
    :
    AbstractCodeBlockNode()
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}
