#pragma once
#include "Node/AbstractCodeBlockNode.h"

namespace Nodable
{
    // Forward declarations
    class InstructionNode;

    enum class Layout {
        ROW,
        COLUMN,
        AUTO,
        DEFAULT = AUTO
    };

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
        [[nodiscard]] const std::vector<InstructionNode*>& getInstructions() const;
        [[nodiscard]] inline Layout getLayout() const { return layout; }
                      inline void   setLayout(Layout _layout) { layout = _layout; }
    private:
        Layout layout;

    /** reflect class using mirror */
    MIRROR_CLASS(CodeBlockNode)
    (
        MIRROR_PARENT(Node)
        MIRROR_PARENT(AbstractCodeBlockNode)
    )

    };
}
