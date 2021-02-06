#include "CodeBlock.h"

using namespace Nodable;

AbstractCodeBlock::AbstractCodeBlock(ScopedCodeBlock *_parent)
{
    parent = _parent;

    if( parent )
    {
        parent->innerBlocs.push_back(this);
    }
}

ScopedCodeBlock::~ScopedCodeBlock()
{
    clear();
}

void ScopedCodeBlock::clear()
{
    for(auto& each: innerBlocs)
    {
        delete each;
    }
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

CodeBlock *ScopedCodeBlock::getLastCodeBlock()
{
    // TODO: mode this somewhere
    // (we put instruction into the last code block

    if ( innerBlocs.empty())
    {
        innerBlocs.push_back(new CodeBlock(this));
    }
    auto block = dynamic_cast<CodeBlock*>(innerBlocs.back());

    return block;
}

CodeBlock::~CodeBlock()
{
    clear();
}

void CodeBlock::clear()
{
    // a code block do NOT owns its instructions nodes
    instructionNodes.clear();
}

bool CodeBlock::hasInstructions() const
{
    return !instructionNodes.empty();
}

InstructionNode* CodeBlock::getFirstInstruction()
{
    return instructionNodes.front();
}
