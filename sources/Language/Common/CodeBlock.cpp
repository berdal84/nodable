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
}

CodeBlock::~CodeBlock()
{
    clear();
}

void CodeBlock::clear()
{
    for(auto& each: instructions)
    {
        delete each;
    }
    instructions.clear();
}
