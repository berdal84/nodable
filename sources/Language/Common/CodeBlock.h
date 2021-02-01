#pragma once
#include <vector>
#include "Instruction.h"
#include "mirror.h"

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
        ScopedCodeBlock* parent;

        MIRROR_CLASS(AbstractCodeBlock)()
    };

    /**
     * A Scoped code block able to contain inner code blocs.
     */
    class ScopedCodeBlock: public AbstractCodeBlock
    {
    public:
        explicit ScopedCodeBlock(ScopedCodeBlock* _parent): AbstractCodeBlock(_parent){}
        ~ScopedCodeBlock() override;
        void clear();
        std::vector<AbstractCodeBlock*> innerBlocs;
        MIRROR_CLASS(ScopedCodeBlock)
        (
            MIRROR_PARENT(AbstractCodeBlock)
        )
    };

    /**
     * A Code block class to contain a set of instructions.
     * This class can't contain other Code blocks.
     */
    class CodeBlock: public AbstractCodeBlock
    {
    public:
        explicit CodeBlock(ScopedCodeBlock* _parent): AbstractCodeBlock(_parent){}
        ~CodeBlock() override;
        void clear();
        std::vector<Instruction*> instructions;
        MIRROR_CLASS(CodeBlock)
        (
            MIRROR_PARENT(AbstractCodeBlock)
        )
    };
}
