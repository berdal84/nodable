#pragma once
#include <vector>
#include <Node/InstructionNode.h>
#include <Language/Common/CodeBlock.h>
#include "mirror.h"
#include "VariableNode.h"

namespace Nodable
{
    class ScopedCodeBlockNode: public Node, public ScopedCodeBlock
    {
        explicit ScopedCodeBlockNode(ScopedCodeBlock* _parent):
            Node("Scoped Block Code"),
            ScopedCodeBlock(_parent){}

        ~ScopedCodeBlockNode() = default;
    };
}
