#pragma once
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/Reflect.h>

namespace Nodable
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
        REFLECT_WITH_INHERITANCE(ScopedCodeBlockNode)
        REFLECT_INHERITS(ProgramNode)
        REFLECT_END
    };
}

