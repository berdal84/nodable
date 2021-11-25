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

        void clear() override;
        [[nodiscard]] bool             hasInstructions() const override;
        [[nodiscard]] InstructionNode* getFirstInstruction() const override;

        REFLECT_DERIVED(CodeBlockNode)
          REFLECT_EXTENDS(AbstractCodeBlockNode)
        REFLECT_END
    };
}
