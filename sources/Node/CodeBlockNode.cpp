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


CodeBlockNode::CodeBlockNode(ScopedCodeBlockNode *_parent)
    :
    AbstractCodeBlockNode(_parent)
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}

void CodeBlockNode::pushInstruction(InstructionNode* _node) {
    this->addChild(_node);
}

const std::vector<InstructionNode*>& CodeBlockNode::getInstructions() const {
    return reinterpret_cast<const std::vector<InstructionNode*>&>( this->children );
}


