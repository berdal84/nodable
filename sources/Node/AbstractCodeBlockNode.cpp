#include "AbstractCodeBlockNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include <Node/InstructionNode.h>
#include <algorithm>

using namespace Nodable;

void AbstractCodeBlockNode::setParent(ScopedCodeBlockNode *_scope)
{
    assert(this->parent == nullptr); // TODO: Parent can't be set twice
    this->parent = _scope;
    _scope->addChild(this);
}

ScopedCodeBlockNode *AbstractCodeBlockNode::getParent()
{
    if ( this->parent ) {
        return this->parent->as<ScopedCodeBlockNode>();
    }
    return nullptr;
}
