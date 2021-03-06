#pragma once
#include <vector>
#include "mirror.h"
#include "Node/Node.h" // base class

namespace Nodable
{
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
        explicit AbstractCodeBlockNode() = default;
    public:
        virtual ~AbstractCodeBlockNode() = default;
        virtual void clear() = 0;
        [[nodiscard]] virtual bool             hasInstructions() const = 0;
        [[nodiscard]] virtual InstructionNode* getFirstInstruction() const = 0;
        [[nodiscard]] virtual VariableNode*    findVariable(std::string _name) { return nullptr; };
        virtual void setParent(ScopedCodeBlockNode *_scope);
        virtual ScopedCodeBlockNode* getParent();

    protected:
        /** reflect class using mirror */
        MIRROR_CLASS(AbstractCodeBlockNode)
        (
            MIRROR_PARENT(Node)
        )
    };
}
