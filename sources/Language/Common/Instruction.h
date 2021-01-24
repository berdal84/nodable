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
        Token* endOfInstructionToken = nullptr;
        bool hasEndOfLine = false;

        // TODO: move this somewhere else, this class should be abstract from Members/Nodes etc....
        Member* result = nullptr;

        std::string suffix;
    };
}
