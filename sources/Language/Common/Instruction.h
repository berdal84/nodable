#pragma once
#include <string>
#include <Core/Token.h>

namespace Nodable
{
    // forward declaration
    class Member;

    /**
     * Simple structure to store an instruction
     */
    struct Instruction
    {
        /** End of instruction token */
        Token* endOfInstructionToken = nullptr;

        /** Pointer to the graph root of this instruction, this is the link between this instruction and Nodable */
        Member* nodeGraphRoot = nullptr;
    };
}
