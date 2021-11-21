#pragma once
#include <nodable/AbstractCodeBlockNode.h>

namespace Nodable
{
    // Forward declarations
    class InstructionNode;

    /**
     * A Code block class to contain a set of instructions.
     * This class can't contain other Code blocks.
     */
    class CodeBlockNode: public AbstractCodeBlockNode
    {
    public:
        explicit CodeBlockNode();
        ~CodeBlockNode();

        void clear();
        [[nodiscard]] bool             hasInstructions() const;
        [[nodiscard]] InstructionNode* getFirstInstruction() const;

        REFLECT_WITH_INHERITANCE(CodeBlockNode, AbstractCodeBlockNode)
    };
}
