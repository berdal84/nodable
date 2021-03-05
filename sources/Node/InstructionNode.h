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
        InstructionNode(const char* _label, CodeBlockNode* _parent);
        ~InstructionNode(){};

        Member* value()const
        {
            return get("value");
        }

        void setValue(Member* _value)
        {
            get("value")->set(_value);
        };

        std::string getTypeAsString()const;

        /** End of instruction token */
        Token* endOfInstructionToken = nullptr;

        /** reflect class using mirror */
        MIRROR_CLASS(InstructionNode)
        (
            MIRROR_PARENT(Node)
        );
    };
}
