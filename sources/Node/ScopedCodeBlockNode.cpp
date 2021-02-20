#include "ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/VariableNode.h"
#include "Node/CodeBlockNode.h"
#include <cstring>
#include <algorithm> // for std::find_if

using namespace Nodable;

ScopedCodeBlockNode::~ScopedCodeBlockNode()
{
    clear();
}

void ScopedCodeBlockNode::clear()
{
    innerBlocs.clear();
    variables.clear();
}

bool ScopedCodeBlockNode::hasInstructions() const
{
    auto found = std::find_if(
            innerBlocs.begin(),
            innerBlocs.end(),
            [](AbstractCodeBlockNode* block )
            {
                return block->hasInstructions();
            });

    return found != innerBlocs.end();
}

InstructionNode *ScopedCodeBlockNode::getFirstInstruction()
{
    auto found = std::find_if(
            innerBlocs.begin(),
            innerBlocs.end(),
            [](AbstractCodeBlockNode* block )
            {
                return block->hasInstructions();
            });

    if ( found != innerBlocs.end())
    {
        return (*found)->getFirstInstruction();
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
    return innerBlocs.back();
}

void ScopedCodeBlockNode::add(AbstractCodeBlockNode* _block)
{
    assert(std::find(innerBlocs.begin(), innerBlocs.end(), _block) == innerBlocs.end() ); // can be added only once
    this->innerBlocs.push_back(_block);
    _block->setParent(this);
}

bool ScopedCodeBlockNode::isEmpty()
{
    return innerBlocs.empty();
}

InstructionNode *ScopedCodeBlockNode::getLastInstruction()
{
    return getLastCodeBlock()->as<CodeBlockNode>()->instructionNodes.back();
}

ScopedCodeBlockNode::ScopedCodeBlockNode(ScopedCodeBlockNode *_parent)
    :
        AbstractCodeBlockNode(_parent)
{
    this->setLabel("unnamed ScopedCodeBlockNode");
}

