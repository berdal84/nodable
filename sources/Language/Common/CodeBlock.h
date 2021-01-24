#pragma once
#include <vector>
#include "Instruction.h"
#include "mirror.h"

namespace Nodable
{
    // forward declarations
    class Scope;

    class CodeBlock
    {
    protected:
        explicit CodeBlock(Scope* _parent );
    public:
        virtual ~CodeBlock() = default;;
        Scope* parent;

        MIRROR_CLASS(CodeBlock)()
    };

    class Scope: public CodeBlock
    {
    public:
        std::vector<CodeBlock*> innerBlocs;
        explicit Scope(Scope* _parent): CodeBlock(_parent){}
        virtual ~Scope();
        void clear();

        MIRROR_CLASS(Scope)
        (
            MIRROR_PARENT(CodeBlock)
        )
    };

    class InstructionBlock: public CodeBlock
    {
    public:
        std::vector<Instruction*> instructions;
        explicit InstructionBlock(Scope* _parent): CodeBlock(_parent){}
        virtual ~InstructionBlock();
        void clear();

        MIRROR_CLASS(InstructionBlock)
        (
            MIRROR_PARENT(CodeBlock)
        )
    };
}
