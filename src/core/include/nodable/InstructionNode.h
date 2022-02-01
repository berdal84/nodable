#pragma once

#include <string>

#include <nodable/Nodable.h> // forward declarations and common stuff
#include <nodable/Node.h> // base class
#include <nodable/Member.h>

namespace Nodable
{
    // forward declarations
    struct Token;

    /*
        The role of this class is to symbolize an instruction.
        The result of the instruction is value()
    */
    class InstructionNode : public Node
    {
    public:
        explicit InstructionNode(const char* _label);
        ~InstructionNode()= default;

        [[nodiscard]] inline Member* value()const { return m_props.get("value"); }
        [[nodiscard]] inline Token*  end_of_instr_token()const { return m_end_of_instr_token; }
                      inline void    end_of_instr_token(Token* token) { m_end_of_instr_token = token; }

    private:
        Token* m_end_of_instr_token = nullptr;
        REFLECT_DERIVED(InstructionNode)
        REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}
