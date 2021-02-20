#include "CodeBlockNode.h"
#include "ScopedCodeBlockNode.h"
#include <algorithm> // for std::find
#include <cstring>

using namespace Nodable;

CodeBlockNode::~CodeBlockNode()
{
    auto found = std::find( parent->innerBlocs.begin(), parent->innerBlocs.end(), this);
    parent->innerBlocs.erase( found );
    clear();
}

void CodeBlockNode::clear()
{
    // a code block do NOT owns its instructions nodes
    instructionNodes.clear();
}

bool CodeBlockNode::hasInstructions() const
{
    return !instructionNodes.empty();
}

InstructionNode* CodeBlockNode::getFirstInstruction()
{
    return instructionNodes.front();
}

void CodeBlockNode::pushInstruction(InstructionNode *_node)
{
    this->instructionNodes.push_back(_node);
}

CodeBlockNode::CodeBlockNode(ScopedCodeBlockNode *_parent)
    :
    AbstractCodeBlockNode(_parent)
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}

