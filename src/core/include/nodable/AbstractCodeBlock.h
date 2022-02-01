#pragma once
#include <vector>

#include <nodable/Reflect.h>

namespace Nodable
{

    // forward declarations
    class InstructionNode;
    class VariableNode;
    class Node;

    class AbstractCodeBlock
    {
    public:
        virtual void                   clear() = 0;
        virtual void                   get_last_instructions(std::vector<InstructionNode *> &out) = 0;
        REFLECT(AbstractCodeBlock)
    };
}
