#include "AbstractCodeBlockNode.h" // abstract base class

namespace Nodable
{
    // Forward declarations
    class InstructionNode;
    class CodeBlockNode;

    /**
     * A Scoped code block contains:
     * - AbstractCodeBlocks (CodeBlock or ScopedCodeBlock)
     * - VariableNodes
     * All of them are NOT owned by this class.
     *
     * For now everything is public because it is WIP.
     */
    class ScopedCodeBlockNode: public AbstractCodeBlockNode
    {
    public:
        explicit ScopedCodeBlockNode() = default;
        ~ScopedCodeBlockNode();
        void clear() override;
        [[nodiscard]] bool isEmpty();
        [[nodiscard]] bool hasInstructions() const override;
        [[nodiscard]] InstructionNode* getFirstInstruction() const override;
        [[nodiscard]] VariableNode* findVariable(std::string _name) override;
        [[nodiscard]] AbstractCodeBlockNode* getLastCodeBlock();
        [[nodiscard]] InstructionNode *getLastInstruction();

        std::vector<VariableNode*> variables;

        /** Reflect class */
        MIRROR_CLASS(ScopedCodeBlockNode)
        (
            MIRROR_PARENT(AbstractCodeBlockNode)
        )
    };
}
