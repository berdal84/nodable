#include "ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/VariableNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/AbstractCodeBlockNode.h"
#include <cstring>
#include <algorithm> // for std::find_if

using namespace Nodable;

ScopedCodeBlockNode::~ScopedCodeBlockNode()
{
    clear();
}

void ScopedCodeBlockNode::clear()
{
    getChildren().clear();
    variables.clear();
}

bool ScopedCodeBlockNode::hasInstructions() const
{
    return getFirstInstruction() != nullptr;
}

InstructionNode *ScopedCodeBlockNode::getFirstInstruction() const
{
    auto found = std::find_if(
            children.begin(),
            children.end(),
            [](Node* block )
            {
                return block->as<AbstractCodeBlockNode>()->hasInstructions();
            });

    if ( found != children.end())
    {
        return (*found)->as<AbstractCodeBlockNode>()->getFirstInstruction();
    }
    return nullptr;
}

VariableNode* ScopedCodeBlockNode::findVariable(std::string _name)
{
    VariableNode* result = nullptr;

    auto findFunction = [_name](const VariableNode* _variable ) -> bool
    {
        return strcmp(_variable->getName(), _name.c_str()) == 0;
    };

    auto it = std::find_if(variables.begin(), variables.end(), findFunction);
    if (it != variables.end()){
        result = *it;
    }

    return result;
}

AbstractCodeBlockNode *ScopedCodeBlockNode::getLastCodeBlock()
{
    auto back = children.back();
    if ( back )
    {
        return back->as<AbstractCodeBlockNode>();
    }
    return nullptr;
}

void ScopedCodeBlockNode::add(AbstractCodeBlockNode* _block)
{
    assert(std::find(children.begin(), children.end(), _block) == children.end() ); // can be added only once
    this->addChild(_block);
    _block->setParent(this);
}

bool ScopedCodeBlockNode::isEmpty()
{
    return children.empty();
}

InstructionNode *ScopedCodeBlockNode::getLastInstruction()
{
    return getLastCodeBlock()->as<CodeBlockNode>()
            ->getInstructions().back()->as<InstructionNode>();
}

ScopedCodeBlockNode::ScopedCodeBlockNode(ScopedCodeBlockNode *_parent)
    :
        AbstractCodeBlockNode(_parent)
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}
