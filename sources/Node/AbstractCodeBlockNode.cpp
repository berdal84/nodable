#include "AbstractCodeBlockNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include <Node/InstructionNode.h>
#include <algorithm>

using namespace Nodable;

ScopedCodeBlockNode *AbstractCodeBlockNode::getParent()
{
    if ( this->parent ) {
        return this->parent->as<ScopedCodeBlockNode>();
    }
    return nullptr;
}
