#include "AbstractCodeBlockNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include <Node/InstructionNode.h>
#include <algorithm>

using namespace Nodable;

AbstractCodeBlockNode::AbstractCodeBlockNode(ScopedCodeBlockNode *_parent)
{
    parent = _parent;

    if( parent )
    {
        parent->innerBlocs.push_back(this);
    }
}

void AbstractCodeBlockNode::setParent(ScopedCodeBlockNode *_scope)
{
    assert(this->parent == nullptr); // Parent can't be set once
    this->parent = _scope;
}

ScopedCodeBlockNode *AbstractCodeBlockNode::getParent()
{
    return this->parent;
}
