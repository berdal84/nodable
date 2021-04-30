#include "AbstractCodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include <Node/InstructionNode.h>

using namespace Nodable;

AbstractCodeBlockNode::AbstractCodeBlockNode()
{
    setNextMaxCount(1);
    setPrevMaxCount(-1);
}

ScopedCodeBlockNode *AbstractCodeBlockNode::getParent()
{
    if ( this->m_parent ) {
        return this->m_parent->as<ScopedCodeBlockNode>();
    }
    return nullptr;
}

void AbstractCodeBlockNode::getLastInstructions(std::vector<InstructionNode*> &out)
{
    if( m_children.empty())
        return;

    Node* last = m_children.back();
    if ( last )
    {
        if ( auto* instr = last->as<InstructionNode>())
        {
            out.push_back(instr);
        }
        else if (auto* code_block = last->as<AbstractCodeBlockNode>())
        {
            code_block->getLastInstructions(out);
        }
    }
}
