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
        explicit CodeBlockNode(ScopedCodeBlockNode* _parent);
        ~CodeBlockNode();

        public:
            void clear();
            [[nodiscard]] bool             hasInstructions() const;
            [[nodiscard]] InstructionNode* getFirstInstruction();
            std::vector<InstructionNode*> instructionNodes;

        MIRROR_CLASS(CodeBlockNode)
        (
            MIRROR_PARENT(Node)
            MIRROR_PARENT(AbstractCodeBlockNode)
        )

        void pushInstruction(InstructionNode *pNode);
    };
}
