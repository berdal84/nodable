#pragma once
#include "Node.h"
#include "ScopedCodeBlockNode.h"

namespace Nodable
{
    // Forward declarations
    class ScopedCodeBlock;
    class InstructionNode;

    /**
     * A Code block class to contain a set of instructions.
     * This class can't contain other Code blocks.
     */
    class CodeBlockNode: public AbstractCodeBlock, public Node
    {
    public:
        explicit CodeBlockNode(ScopedCodeBlock* _parent): AbstractCodeBlock(_parent){}
        ~CodeBlockNode();

        public:
            void clear();
            [[nodiscard]] bool             hasInstructions() const;
            [[nodiscard]] InstructionNode* getFirstInstruction();
            std::vector<InstructionNode*> instructionNodes;

        MIRROR_CLASS(CodeBlockNode)
        (
            MIRROR_PARENT(Node)
            MIRROR_PARENT(AbstractCodeBlock)
        )
    };
}
