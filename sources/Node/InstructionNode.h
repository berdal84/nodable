#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>

namespace Nodable
{
    // forward declarations
    class Token;
    class CodeBlockNode;

    /*
        The role of this class is to symbolize an instruction.
        The result of the instruction is value()
    */
    class InstructionNode : public Node
    {
    public:
        explicit InstructionNode(const char* _label);
        ~InstructionNode()= default;;

        [[nodiscard]] Member* getValue()const { return props.get("value"); }
        void setValue(Member* _value) const { getValue()->set(_value); };

        /** End of instruction token */
        Token* endOfInstructionToken = nullptr;

        /** reflect class using mirror */
        MIRROR_CLASS(InstructionNode)
        (
            MIRROR_PARENT(Node)
        );
    };
}
