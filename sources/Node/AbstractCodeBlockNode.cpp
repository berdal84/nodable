#include "AbstractCodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include <Node/InstructionNode.h>

using namespace Nodable;

ScopedCodeBlockNode *AbstractCodeBlockNode::getParent()
{
    if ( this->m_parent ) {
        return this->m_parent->as<ScopedCodeBlockNode>();
    }
    return nullptr;
}
