#include <nodable/CodeBlockNode.h>
#include <nodable/InstructionNode.h>

using namespace Nodable::core;

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
    m_children.clear();
}

bool CodeBlockNode::hasInstructions() const
{
    return !m_children.empty();
}

InstructionNode* CodeBlockNode::getFirstInstruction() const
{
    return m_children.front()->as<InstructionNode>();
}
