#include "CodeBlock.h"

using namespace Nodable;

CodeBlock::CodeBlock(Scope *_parent)
{
    parent = _parent;

    if( parent )
    {
        parent->innerBlocs.push_back(this);
    }
}

Scope::~Scope()
{
    clear();
}

void Scope::clear()
{
    for(auto& each: innerBlocs)
    {
        delete each;
    }
    innerBlocs.clear();
}

InstructionBlock::~InstructionBlock()
{
    clear();
}

void InstructionBlock::clear()
{
    for(auto& each: instructions)
    {
        delete each;
    }
    instructions.clear();
}
