#pragma once
#include <nodable/ScopedCodeBlockNode.h>

namespace Nodable::core
{
    /**
     * Class to define a program inside Nodable.
     * A program can be loaded by the Nodable::Runner
     */
    class ProgramNode: public ScopedCodeBlockNode
    {
    public:
        ProgramNode() = default;
        ~ProgramNode(){}
    };
}

