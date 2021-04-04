#pragma once
#include "Node/ScopedCodeBlockNode.h"

namespace Nodable
{
    /**
     * Class to define a program inside Nodable.
     * A program can be loaded by the Nodable::VM
     */
    class ProgramNode: public ScopedCodeBlockNode
    {
    public:
        ProgramNode(){}
        ~ProgramNode(){}
    };
}

