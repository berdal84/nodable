#include "CodeBlockNode.h"

using namespace Nodable;

AbstractCodeBlock::AbstractCodeBlock(ScopedCodeBlock *_parent)
{
    parent = _parent;

    if( parent )
    {
        parent->innerBlocs.push_back(this);
    }
}

void AbstractCodeBlock::setParent(ScopedCodeBlock *_scope)
{
    assert(this->parent == nullptr); // Parent can't be set once
    this->parent = _scope;
}

ScopedCodeBlock *AbstractCodeBlock::getParent()
{
    return nullptr;
}

ScopedCodeBlock::~ScopedCodeBlock()
{
    clear();
}

void ScopedCodeBlock::clear()
{
    innerBlocs.clear();
    variables.clear();
}

bool ScopedCodeBlock::hasInstructions() const
{
    auto found = std::find_if(
            innerBlocs.begin(),
            innerBlocs.end(),
            [](AbstractCodeBlock* block )
            {
                return block->hasInstructions();
            });

    return found != innerBlocs.end();
}

InstructionNode *ScopedCodeBlock::getFirstInstruction()
{
    auto found = std::find_if(
            innerBlocs.begin(),
            innerBlocs.end(),
            [](AbstractCodeBlock* block )
            {
                return block->hasInstructions();
            });

    if ( found != innerBlocs.end())
    {
        return (*found)->getFirstInstruction();
    }
    return nullptr;
}

VariableNode* ScopedCodeBlock::findVariable(std::string _name)
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

CodeBlockNode *ScopedCodeBlock::getLastCodeBlock()
{
    return dynamic_cast<CodeBlockNode*>(innerBlocs.back());
}

void ScopedCodeBlock::add(CodeBlockNode* _block)
{
    assert(std::find(innerBlocs.begin(), innerBlocs.end(), _block) == innerBlocs.end() ); // can be added only once
    this->innerBlocs.push_back(_block);
    _block->setParent(this);
}

bool ScopedCodeBlock::isEmpty()
{
    return innerBlocs.empty();
}

InstructionNode *ScopedCodeBlock::getLastInstruction()
{
    return getLastCodeBlock()->instructionNodes.back();
}
