#pragma once
#include <vector>
#include <mirror.h>
#include <nodable/Node.h> // base class

namespace Nodable
{
    enum class Layout {
        ROW,
        COLUMN,
        AUTO,
        DEFAULT = AUTO
    };

    // forward declarations
    class ScopedCodeBlockNode;
    class InstructionNode;
    class VariableNode;

    /**
     * Code bloc abstract class define an interface for ScopedCodeBlockNodes and CodeBlockNodes
     */
    class AbstractCodeBlockNode: public Node
    {
    protected:
        explicit AbstractCodeBlockNode();
    public:
        ~AbstractCodeBlockNode() override = default;

                      virtual void                 clear() = 0;
        [[nodiscard]] virtual bool                 hasInstructions() const = 0;
        [[nodiscard]] virtual InstructionNode*     getFirstInstruction() const = 0;
                      virtual void                 getLastInstructions(std::vector<InstructionNode *>& out);
        [[nodiscard]] virtual VariableNode*        findVariable(const std::string& _name) { return nullptr; }
        [[nodiscard]] virtual ScopedCodeBlockNode* getParent();
        [[nodiscard]] inline Layout                getLayout() const { return layout; }

                      inline void                  setLayout(Layout _layout) { layout = _layout; }
    private:
        Layout layout = Layout::DEFAULT;

        /** reflect class using mirror */
        MIRROR_CLASS(AbstractCodeBlockNode)
        (
            MIRROR_PARENT(Node)
        )
    };
}
