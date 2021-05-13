#pragma once
#include "Node/AbstractCodeBlockNode.h"

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

    /** reflect class using mirror */
    MIRROR_CLASS(CodeBlockNode)
    (
        MIRROR_PARENT(AbstractCodeBlockNode)
    )

    };
}
