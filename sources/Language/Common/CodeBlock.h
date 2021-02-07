#pragma once
#include <vector>
#include <Node/InstructionNode.h>
#include "mirror.h"
#include "VariableNode.h"

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlock;

    /**
     * Code bloc abstract class as foundation for Scoped and Unscoped instruction blocs
     */
    class AbstractCodeBlock
    {
    protected:
        explicit AbstractCodeBlock(ScopedCodeBlock* _parent );
    public:
        virtual ~AbstractCodeBlock() = default;
        virtual void clear() = 0;
        [[nodiscard]] virtual bool             hasInstructions() const = 0;
        [[nodiscard]] virtual InstructionNode* getFirstInstruction() = 0;
        [[nodiscard]] virtual VariableNode*    findVariable(std::string _name) { return nullptr; };
        void setParent(ScopedCodeBlock *_scope);
        ScopedCodeBlock* getParent();

    protected:
        ScopedCodeBlock* parent;

        MIRROR_CLASS(AbstractCodeBlock)()
    };

    /**
     * A Scoped code block able to contain inner code blocs.
     */
    class CodeBlockNode;
    class ScopedCodeBlock: public AbstractCodeBlock
    {
    public:
        explicit ScopedCodeBlock(ScopedCodeBlock* _parent): AbstractCodeBlock(_parent){}
        ~ScopedCodeBlock() override;
        void clear();
        void add(CodeBlockNode*);
        [[nodiscard]] bool             hasInstructions() const;
        [[nodiscard]] InstructionNode* getFirstInstruction();
        [[nodiscard]] VariableNode*    findVariable(std::string _name);
        [[nodiscard]] CodeBlockNode*       getLastCodeBlock();
        std::vector<AbstractCodeBlock*> innerBlocs;
        std::vector<VariableNode*>      variables;

        MIRROR_CLASS(ScopedCodeBlock)
        (
            MIRROR_PARENT(AbstractCodeBlock)
        )

        bool isEmpty();

        InstructionNode *getLastInstruction();
    };

}
